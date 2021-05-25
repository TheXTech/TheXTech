#include "../globals.h"
#include "../graphics.h"
#include "../layers.h"
#include "../npc.h"
#include "../collision.h"
#include "../editor/editor.h"
#ifdef NEW_EDITOR
#include "../editor/new_editor.h"
#endif

void DrawEditorLevel(int Z)
{
    int A = 0;
    int B = 0;
    int C = 0;
    int D;
    int E;
    Location_t tempLocation;
    int S = 0; // Level section to display

#ifdef __3DS__
    frmMain.setLayer(2);
#endif
    if(LevelEditor)
    {
        BlockFlash += 1;

        if(BlockFlash > 45)
            BlockFlash = 0;

        if(BlockFlash <= 30)
        {
            for (A = 1; A <= numBlock; A++)
            {
                if(Block[A].Special > 0 && !Block[A].Hidden)
                {
                    if(vScreenCollision(Z, Block[A].Location))
                    {
                        if (Block[A].Special > 1000)
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
                        frmMain.renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[C],
                            vScreenY[Z] + tempLocation.Y + NPCFrameOffsetY[C],
                            tempLocation.Width, tempLocation.Height,
                            GFXNPC[C], 0, EditorNPCFrame(C, -1) * tempLocation.Height);
                    }
                }
            }

            for (A = 1; A <= numNPCs; A++)
            {
                if(!NPC[A].Hidden && (NPC[A].Type == 91 || NPC[A].Type == 96)
                    && (NPC[A].Special > 0))
                {
                    if (vScreenCollision(Z, NPC[A].Location))
                    {
                        C = NPC[A].Special;
                        if (NPCWidthGFX[C] == 0)
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
                        frmMain.renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[C],
                            vScreenY[Z] + tempLocation.Y + NPCFrameOffsetY[C],
                            tempLocation.Width, tempLocation.Height,
                            GFXNPC[C], 0, EditorNPCFrame(C, -1) * tempLocation.Height);
                    }
                }
            }
        }


        for(A = 1; A <= 2; A++)
        {
            if (!(PlayerStart[A].Width > 0)) continue;
            if (vScreenCollision(Z, PlayerStart[A]))
            {
                C = Physics.PlayerHeight[testPlayer[A].Character][2] - Physics.PlayerHeight[A][2];
                switch (testPlayer[A].Character)
                {
                case 1:
                    frmMain.renderTexture(vScreenX[Z] + PlayerStart[A].X + MarioFrameX[201],
                            vScreenY[Z] + PlayerStart[A].Y + MarioFrameY[201] - C,
                            99, 99, GFXMario[2], 500, 0);
                    break;
                case 2:
                    frmMain.renderTexture(vScreenX[Z] + PlayerStart[A].X + LuigiFrameX[201],
                            vScreenY[Z] + PlayerStart[A].Y + LuigiFrameY[201] - C,
                            99, 99, GFXLuigi[2], 500, 0);
                    break;
                case 3:
                    frmMain.renderTexture(vScreenX[Z] + PlayerStart[A].X + PeachFrameX[201],
                            vScreenY[Z] + PlayerStart[A].Y + PeachFrameY[201] - C,
                            99, 99, GFXPeach[2], 500, 0);
                    break;
                case 4:
                    frmMain.renderTexture(vScreenX[Z] + PlayerStart[A].X + ToadFrameX[201],
                            vScreenY[Z] + PlayerStart[A].Y + ToadFrameY[201] - C,
                            99, 99, GFXToad[2], 500, 0);
                    break;
                case 5:
                    frmMain.renderTexture(vScreenX[Z] + PlayerStart[A].X + LinkFrameX[201],
                            vScreenY[Z] + PlayerStart[A].Y + LinkFrameY[201] - C,
                            99, 99, GFXLink[2], 500, 0);
                    break;
                }
            }
        }

        for(A = 1; A <= numWarps; A++)
        {
            if(Warp[A].Direction > 0 && !Warp[A].Hidden)
            {
                bool complete = Warp[A].PlacedEnt && Warp[A].PlacedExit;
                if(Warp[A].PlacedEnt)
                {
                    if (complete)
                        frmMain.renderRect(vScreenX[Z] + Warp[A].Entrance.X, vScreenY[Z] + Warp[A].Entrance.Y, 32, 32,
                            1.f, 0.f, 1.f, 1.f, false);
                    else
                        frmMain.renderRect(vScreenX[Z] + Warp[A].Entrance.X, vScreenY[Z] + Warp[A].Entrance.Y, 32, 32,
                            1.f, 0.f, 0.f, 1.f, false);
                    SuperPrint(std::to_string(A), 1, vScreenX[Z] + Warp[A].Entrance.X + 2, vScreenY[Z] + Warp[A].Entrance.Y + 2);
                }
                if(Warp[A].PlacedExit)
                {
                    if (complete)
                        frmMain.renderRect(vScreenX[Z] + Warp[A].Exit.X, vScreenY[Z] + Warp[A].Exit.Y, 32, 32,
                            1.f, 0.f, 1.f, 1.f, false);
                    else
                        frmMain.renderRect(vScreenX[Z] + Warp[A].Exit.X, vScreenY[Z] + Warp[A].Exit.Y, 32, 32,
                            1.f, 0.f, 0.f, 1.f, false);
                    SuperPrint(std::to_string(A), 1, vScreenX[Z] + Warp[A].Exit.X + Warp[A].Exit.Width - 16 - 2,
                        vScreenY[Z] + Warp[A].Exit.Y + Warp[A].Exit.Height - 14 - 2);
                }
            }
        }
    }

#ifdef __3DS__
    frmMain.setLayer(0);
#endif
    if(LevelEditor)
    {
        if (BlockFlash > 30 || BlockFlash == 0)
        {
            if (vScreenX[Z] + level[S].X > 0) {
                frmMain.renderRect(0, 0,
                                   vScreenX[Z] + level[S].X, ScreenH, 0.f, 0.f, 0.f, 1.f, true);
            }
            if (ScreenW > level[S].Width + vScreenX[Z]) {
                frmMain.renderRect(level[S].Width + vScreenX[Z], 0,
                                   ScreenW - (level[S].Width + vScreenX[Z]), ScreenH, 0.f, 0.f, 0.f, 1.f, true);
            }
            if (vScreenY[Z] + level[S].Y > 0) {
                frmMain.renderRect(0, 0,
                                   ScreenW, vScreenY[Z] + level[S].Y, 0.f, 0.f, 0.f, 1.f, true);
            }
            if (ScreenH > level[S].Height + vScreenY[Z]) {
                frmMain.renderRect(0, level[S].Height + vScreenY[Z],
                                   ScreenW, ScreenH - (level[S].Height + vScreenY[Z]), 0.f, 0.f, 0.f, 1.f, true);
            }
        }
    }

#ifdef __3DS__
    frmMain.setLayer(3);
#endif
    // if (editorScreen.active && !MessageText.empty())
    if(!MessageText.empty()) // In-Editor message box preview
    {
        DrawMessage();
    }

    // Display the cursor
    {
        auto &e = EditorCursor;
        int curX = int(double(e.X) - vScreen[Z].Left);
        int curY = int(double(e.Y) - vScreen[Z].Top);

        if (BlockFlash < 15)
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

                            frmMain.renderTexture(vScreenX[Z] + b.Location.X + C * 32,
                                                  vScreenY[Z] + b.Location.Y + B * 32,
                                                  32, 32, GFXBlock[b.Type], D * 32, E * 32);
//                                    if((D == 0 || D == 2) || (E == 0 || E == 2))
//                                    {
//                                        frmMain.renderTexture(vScreenX(Z) + .Location.X + C * 32, vScreenY(Z) + .Location.Y + B * 32, 32, 32, GFXBlock(.Type), D * 32, E * 32)
//                                    }
//                                    else
//                                        frmMain.renderTexture(vScreenX(Z) + .Location.X + C * 32, vScreenY(Z) + .Location.Y + B * 32, 32, 32, GFXBlock(.Type), D * 32, E * 32)

                        }
                    }
                }
            }
            else
            {
                if(vScreenCollision(Z, b.Location))
                {
                    frmMain.renderTexture(vScreenX[Z] + b.Location.X,
                                          vScreenY[Z] + b.Location.Y + b.ShakeY3,
                                          b.Location.Width,
                                          b.Location.Height,
                                          GFXBlock[b.Type], 0, BlockFrame[b.Type] * 32);
                }
            }
        }

        else if(e.Mode == OptCursor_t::LVL_SETTINGS) // Player start points
        {
            // TODO: determine what is going on here
            if(e.SubMode == 4 || e.SubMode == 5)
            {
                A = e.SubMode - 3;
                C = Physics.PlayerHeight[testPlayer[A].Character][2] - Physics.PlayerHeight[A][2];
                switch (testPlayer[A].Character)
                {
                case 1:
                    frmMain.renderTexture(vScreenX[Z] + e.Location.X + MarioFrameX[201],
                            vScreenY[Z] + e.Location.Y + MarioFrameY[201] - C,
                            99, 99, GFXMario[2], 500, 0);
                    break;
                case 2:
                    frmMain.renderTexture(vScreenX[Z] + e.Location.X + LuigiFrameX[201],
                            vScreenY[Z] + e.Location.Y + LuigiFrameY[201] - C,
                            99, 99, GFXLuigi[2], 500, 0);
                    break;
                case 3:
                    frmMain.renderTexture(vScreenX[Z] + e.Location.X + PeachFrameX[201],
                            vScreenY[Z] + e.Location.Y + PeachFrameY[201] - C,
                            99, 99, GFXPeach[2], 500, 0);
                    break;
                case 4:
                    frmMain.renderTexture(vScreenX[Z] + e.Location.X + ToadFrameX[201],
                            vScreenY[Z] + e.Location.Y + ToadFrameY[201] - C,
                            99, 99, GFXToad[2], 500, 0);
                    break;
                case 5:
                    frmMain.renderTexture(vScreenX[Z] + e.Location.X + LinkFrameX[201],
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
                frmMain.renderTexture(vScreenX[Z] + b.Location.X,
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
                frmMain.renderTexture(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type],
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

                    frmMain.renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[n.Type],
                                          vScreenY[Z] + tempLocation.Y,
                                          tempLocation.Width, tempLocation.Height,
                                          GFXNPC[n.Special], 0, B * tempLocation.Height);
                }

                frmMain.renderTexture(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] - NPCWidthGFX[n.Type] / 2 + n.Location.Width / 2,
                                      vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] - NPCHeightGFX[n.Type] + n.Location.Height,
                                      NPCWidthGFX[n.Type], NPCHeightGFX[n.Type], GFXNPC[n.Type],
                                      0, n.Frame * NPCHeightGFX[n.Type]);
            }
        }
        else if(EditorCursor.Mode == OptCursor_t::LVL_WATER) // Water
        {
            if (EditorCursor.Water.Quicksand)
                frmMain.renderRect(vScreenX[Z] + EditorCursor.Location.X, vScreenY[Z] + EditorCursor.Location.Y, EditorCursor.Location.Width, EditorCursor.Location.Height,
                    1.f, 1.f, 0.f, 1.f, false);
            else
                frmMain.renderRect(vScreenX[Z] + EditorCursor.Location.X, vScreenY[Z] + EditorCursor.Location.Y, EditorCursor.Location.Width, EditorCursor.Location.Height,
                    0.f, 1.f, 1.f, 1.f, false);
        }
        else if(EditorCursor.Mode == OptCursor_t::LVL_WARPS)
        {
            frmMain.renderRect(vScreenX[Z] + EditorCursor.Location.X, vScreenY[Z] + EditorCursor.Location.Y, EditorCursor.Location.Width, EditorCursor.Location.Height,
                1.f, 0.f, 0.f, 1.f, false);
        }

        if(EditorCursor.Mode == 0 || EditorCursor.Mode == 6) // Eraser
        {
            frmMain.renderTexture(curX - 2, curY, GFX.ECursor[3]);
        }

        else if(EditorCursor.Mode == 13 || EditorCursor.Mode == 14) // Selector
        {
            frmMain.renderTexture(curX, curY, GFX.ECursor[2]);
        }

        // Section Resize
        else if(EditorCursor.Mode == 2 && EditorCursor.SubMode < 4)
        {
            frmMain.renderTexture(curX, curY, GFX.ECursor[1]);
        }

//                Else
        else
        {
//                    If .Mode = 5 Then
            frmMain.renderTexture(curX, curY, GFX.ECursor[2]);
            if(!e.Layer.empty() && strcasecmp(e.Layer.c_str(), "Default") != 0)
                SuperPrint(EditorCursor.Layer, 3, curX + 28 , curY + 34);
        }

//            End With
    }
}

void DrawEditorWorld()
{
    int Z = 1;

    BlockFlash += 1;

    if(BlockFlash > 45)
        BlockFlash = 0;

    if (BlockFlash >= 30)
    {
        // don't draw the currently held object
    }
    else if (EditorCursor.Mode == OptCursor_t::WLD_TILES)
    {
        frmMain.renderTexture(vScreenX[Z] + EditorCursor.Tile.Location.X,
            vScreenY[Z] + EditorCursor.Tile.Location.Y,
            EditorCursor.Tile.Location.Width,
            EditorCursor.Tile.Location.Height,
            GFXTile[EditorCursor.Tile.Type],
            0,
            TileHeight[EditorCursor.Tile.Type] * TileFrame[EditorCursor.Tile.Type]);
    }
    else if (EditorCursor.Mode == OptCursor_t::WLD_SCENES)
    {
        frmMain.renderTexture(vScreenX[Z] + EditorCursor.Scene.Location.X,
            vScreenY[Z] + EditorCursor.Scene.Location.Y,
            EditorCursor.Scene.Location.Width,
            EditorCursor.Scene.Location.Height,
            GFXScene[EditorCursor.Scene.Type],
            0,
            SceneHeight[EditorCursor.Scene.Type] * SceneFrame[EditorCursor.Scene.Type]);
    }
    else if (EditorCursor.Mode == OptCursor_t::WLD_LEVELS)
    {
        if(EditorCursor.WorldLevel.Path == true)
        {
            frmMain.renderTexture(vScreenX[Z] + EditorCursor.WorldLevel.Location.X,
                                  vScreenY[Z] + EditorCursor.WorldLevel.Location.Y,
                                  EditorCursor.WorldLevel.Location.Width,
                                  EditorCursor.WorldLevel.Location.Height,
                                  GFXLevelBMP[0], 0, 0);
        }
        if(EditorCursor.WorldLevel.Path2 == true)
        {
            frmMain.renderTexture(vScreenX[Z] + EditorCursor.WorldLevel.Location.X - 16,
                                  vScreenY[Z] + 8 + EditorCursor.WorldLevel.Location.Y,
                                  64, 32,
                                  GFXLevelBMP[29], 0, 0);
        }
        if(GFXLevelBig[EditorCursor.WorldLevel.Type] == true)
        {
            frmMain.renderTexture(vScreenX[Z] + EditorCursor.WorldLevel.Location.X - (GFXLevelWidth[EditorCursor.WorldLevel.Type] - 32) / 2.0,
                                  vScreenY[Z] + EditorCursor.WorldLevel.Location.Y - GFXLevelHeight[EditorCursor.WorldLevel.Type] + 32,
                                  GFXLevelWidth[EditorCursor.WorldLevel.Type], GFXLevelHeight[EditorCursor.WorldLevel.Type],
                                  GFXLevelBMP[EditorCursor.WorldLevel.Type], 0, 32 * LevelFrame[EditorCursor.WorldLevel.Type]);
        }
        else
        {
            frmMain.renderTexture(vScreenX[Z] + EditorCursor.WorldLevel.Location.X,
                                  vScreenY[Z] + EditorCursor.WorldLevel.Location.Y,
                                  EditorCursor.WorldLevel.Location.Width, EditorCursor.WorldLevel.Location.Height,
                                  GFXLevelBMP[EditorCursor.WorldLevel.Type], 0, 32 * LevelFrame[EditorCursor.WorldLevel.Type]);
        }
    }
    else if (EditorCursor.Mode == OptCursor_t::WLD_PATHS)
    {
        frmMain.renderTexture(vScreenX[Z] + EditorCursor.WorldPath.Location.X,
            vScreenY[Z] + EditorCursor.WorldPath.Location.Y,
            EditorCursor.WorldPath.Location.Width,
            EditorCursor.WorldPath.Location.Height,
            GFXPath[EditorCursor.WorldPath.Type],
            0, 0);
    }
    else if (EditorCursor.Mode == OptCursor_t::WLD_MUSIC)
    {
        frmMain.renderRect(vScreenX[Z] + EditorCursor.WorldMusic.Location.X, vScreenY[Z] + EditorCursor.WorldMusic.Location.Y, 32, 32,
            1.f, 0.f, 1.f, 1.f, false);
        SuperPrint(std::to_string(EditorCursor.WorldMusic.Type), 1, vScreenX[Z] + EditorCursor.WorldMusic.Location.X + 2, vScreenY[Z] + EditorCursor.WorldMusic.Location.Y + 2);
    }
    if (EditorCursor.Mode == OptCursor_t::LVL_ERASER || EditorCursor.Mode == OptCursor_t::LVL_ERASER0)
    {
        frmMain.renderTexture(EditorCursor.X - 2,
            EditorCursor.Y,
            22, 30,
            GFX.ECursor[3], 0, 0);
    }
    else
    {
        frmMain.renderTexture(EditorCursor.X,
            EditorCursor.Y + 8,
            32, 32,
            GFX.ECursor[2], 0, 0);
    }
}