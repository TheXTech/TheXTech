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
#include "../player.h"


// draws GFX to screen when on the world map/world map editor
void UpdateGraphics2()
{
    if(!GameIsActive)
        return;

    float c = ShadowMode ? 0.f : 1.f;
    cycleCount = cycleCount + 1;
    if(FrameSkip == true)
    {
        if(SDL_GetTicks() + floor(1000 * (1 - (cycleCount / 63.0))) > GoalTime)
            return;
    }
    fpsCount = fpsCount + 1;
    int A = 0;
    int B = 0;
    int Z = 0;
    int WPHeight = 0;
//    Location_t tempLocation;
    Z = 1;

    vScreen[Z].Left = 0;
    vScreen[Z].Top = 0;
    vScreen[Z].Width = ScreenW;
    vScreen[Z].Height = ScreenH;
    SpecialFrames();
    // Update level frames
    for(A = 1; A <= maxLevelType; A++)
    {
        LevelFrame2[A]++;
        if(LevelFrame2[A] >= LevelFrameSpeed[A])
        {
            LevelFrame2[A] = 0;
            LevelFrame[A]++;
            if(LevelFrame[A] >= LevelFrameCount[A])
            {
                LevelFrame[A] = 0;
            }
        }
    }


    // Update tile frames
    for(A = 1; A <= maxTileType; A++)
    {
        TileFrame2[A]++;
        if(TileFrame2[A] >= TileFrameSpeed[A])
        {
            TileFrame2[A] = 0;
            TileFrame[A]++;
            if(TileFrame[A] >= TileFrameCount[A])
            {
                TileFrame[A] = 0;
            }
        }
    }


    // Update path frames
    for(A = 1; A <= maxPathType; A++)
    {
        PathFrame2[A]++;
        if(PathFrame2[A] >= PathFrameSpeed[A])
        {
            PathFrame2[A] = 0;
            PathFrame[A]++;
            if(PathFrame[A] >= PathFrameCount[A])
            {
                PathFrame[A] = 0;
            }
        }
    }
    // Update scene frames
    for(A = 1; A <= maxSceneType; A++)
    {
        SceneFrame2[A]++;
        if(SceneFrame2[A] >= SceneFrameSpeed[A])
        {
            SceneFrame2[A] = 0;
            SceneFrame[A]++;
            if(SceneFrame[A] >= SceneFrameCount[A])
            {
                SceneFrame[A] = 0;
            }
        }
    }

//    if(WorldEditor == true)
//    {
//        frmMain.renderTexture(0, 0, ScreenW, ScreenH, 0, 0, 0);
//    }
//    else
//    {
//        frmMain.renderTexture(0, 0, ScreenW, ScreenH, 0, 0, 0);
//    }
    frmMain.clearBuffer();

//    if(TakeScreen == true)
//    {
//        if(LevelEditor == true || MagicHand == true)
//            frmLevelWindow::vScreen[1].AutoRedraw = true;
//        else
//            frmMain.AutoRedraw = true;
//    }

//    If LevelEditor = True Then
//        For A = 1 To numTiles
//            With Tile(A)
//                If vScreenCollision(1, .Location) = True Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXTile(.Type), 0, TileHeight(.Type) * TileFrame(.Type), vbSrcCopy
//                End If
//            End With
//        Next A
//        For A = 1 To numScenes
//            With Scene(A)
//                If vScreenCollision(1, .Location) = True Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXSceneMask(.Type), 0, SceneHeight(.Type) * SceneFrame(.Type), vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXScene(.Type), 0, SceneHeight(.Type) * SceneFrame(.Type), vbSrcPaint
//                End If
//            End With
//        Next A
//        For A = 1 To numWorldPaths
//            With WorldPath(A)
//                If vScreenCollision(1, .Location) = True Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXPathMask(.Type), 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXPath(.Type), 0, 0, vbSrcPaint
//                End If
//            End With
//        Next A
//        For A = 1 To numWorldLevels
//            With WorldLevel(A)
//                If vScreenCollision(1, .Location) = True Then
//                    If .Path = True Then
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXLevelMask(0), 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXLevel(0), 0, 0, vbSrcPaint
//                    End If
//                    If .Path2 = True Then
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 16, vScreenY(Z) + 8 + .Location.Y, 64, 32, GFXLevelMask(29), 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 16, vScreenY(Z) + 8 + .Location.Y, 64, 32, GFXLevel(29), 0, 0, vbSrcPaint
//                    End If
//                    If GFXLevelBig(.Type) = True Then
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X - (GFXLevelWidth(.Type) - 32) / 2, vScreenY(Z) + .Location.Y - GFXLevelHeight(.Type) + 32, GFXLevelWidth(.Type), GFXLevelHeight(.Type), GFXLevelMask(.Type), 0, 32 * LevelFrame(.Type), vbSrcAnd
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X - (GFXLevelWidth(.Type) - 32) / 2, vScreenY(Z) + .Location.Y - GFXLevelHeight(.Type) + 32, GFXLevelWidth(.Type), GFXLevelHeight(.Type), GFXLevel(.Type), 0, 32 * LevelFrame(.Type), vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXLevelMask(.Type), 0, 32 * LevelFrame(.Type), vbSrcAnd
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXLevel(.Type), 0, 32 * LevelFrame(.Type), vbSrcPaint
//                    End If
//                End If
//            End With
//        Next A
//    Else
    {
        for(A = 1; A <= numTiles; A++)
        {
            if(vScreenCollision2(1, Tile[A].Location) == true)
            {
//                frmMain.renderTexture(vScreenX[Z] + Tile[A].Location.X, vScreenY[Z] + Tile[A].Location.Y, Tile[A].Location.Width, Tile[A].Location.Height, GFXTile[Tile[A].Type], 0, TileHeight[Tile[A].Type] * TileFrame[Tile[A].Type]);
                frmMain.renderTexture(vScreenX[Z] + Tile[A].Location.X,
                                      vScreenY[Z] + Tile[A].Location.Y,
                                      Tile[A].Location.Width,
                                      Tile[A].Location.Height,
                                      GFXTileBMP[Tile[A].Type], 0, TileHeight[Tile[A].Type] * TileFrame[Tile[A].Type]);
            }
        }
        for(A = 1; A <= numScenes; A++)
        {
            if(vScreenCollision2(1, Scene[A].Location) == true && Scene[A].Active == true)
            {
//                frmMain.renderTexture(vScreenX[Z] + Scene[A].Location.X, vScreenY[Z] + Scene[A].Location.Y, Scene[A].Location.Width, Scene[A].Location.Height, GFXSceneMask[Scene[A].Type], 0, SceneHeight[Scene[A].Type] * SceneFrame[Scene[A].Type]);
//                frmMain.renderTexture(vScreenX[Z] + Scene[A].Location.X, vScreenY[Z] + Scene[A].Location.Y, Scene[A].Location.Width, Scene[A].Location.Height, GFXScene[Scene[A].Type], 0, SceneHeight[Scene[A].Type] * SceneFrame[Scene[A].Type]);
                frmMain.renderTexture(vScreenX[Z] + Scene[A].Location.X,
                                      vScreenY[Z] + Scene[A].Location.Y,
                                      Scene[A].Location.Width, Scene[A].Location.Height,
                                      GFXSceneBMP[Scene[A].Type], 0, SceneHeight[Scene[A].Type] * SceneFrame[Scene[A].Type]);
            }
        }
        for(A = 1; A <= numWorldPaths; A++)
        {
            if(vScreenCollision2(1, WorldPath[A].Location) == true && WorldPath[A].Active == true)
            {
//                frmMain.renderTexture(vScreenX[Z] + WorldPath[A].Location.X, vScreenY[Z] + WorldPath[A].Location.Y, WorldPath[A].Location.Width, WorldPath[A].Location.Height, GFXPathMask[WorldPath[A].Type], 0, 0);
//                frmMain.renderTexture(vScreenX[Z] + WorldPath[A].Location.X, vScreenY[Z] + WorldPath[A].Location.Y, WorldPath[A].Location.Width, WorldPath[A].Location.Height, GFXPath[WorldPath[A].Type], 0, 0);
                frmMain.renderTexture(vScreenX[Z] + WorldPath[A].Location.X,
                                      vScreenY[Z] + WorldPath[A].Location.Y,
                                      WorldPath[A].Location.Width, WorldPath[A].Location.Height,
                                      GFXPathBMP[WorldPath[A].Type], 0, PathHeight[WorldPath[A].Type] * PathFrame[WorldPath[A].Type]);
            }
        }

        for(A = 1; A <= numWorldLevels; A++)
        {
            if(vScreenCollision2(1, WorldLevel[A].Location) == true && WorldLevel[A].Active == true)
            {
                if(WorldLevel[A].Path == true)
                {
                    frmMain.renderTexture(vScreenX[Z] + WorldLevel[A].Location.X - (GFXLevelWidth[0] - LevelWidth[WorldLevel[A].Type]) * 0.5,
                                          vScreenY[Z] + WorldLevel[A].Location.Y,
                                          GFXLevelWidth[0], GFXLevelHeight[0],
                                          GFXLevelBMP[0], 0, 0);
                }
                if(WorldLevel[A].Path2 == true)
                {
                    frmMain.renderTexture(vScreenX[Z] + WorldLevel[A].Location.X - (GFXLevelWidth[29] - LevelWidth[WorldLevel[A].Type]) * 0.5,
                                          vScreenY[Z] + 8 + WorldLevel[A].Location.Y,
                                          GFXLevelWidth[29], GFXLevelHeight[29],
                                          GFXLevelBMP[29], 0,  GFXLevelHeight[29] * LevelFrame[29]);
                }
                frmMain.renderTexture(vScreenX[Z] + WorldLevel[A].Location.X - (LevelWidth[WorldLevel[A].Type] - 32) / 2.0,
                                      vScreenY[Z] + WorldLevel[A].Location.Y - LevelHeight[WorldLevel[A].Type] + 32,
                                      LevelWidth[WorldLevel[A].Type], LevelHeight[WorldLevel[A].Type],
                                      GFXLevelBMP[WorldLevel[A].Type], 0, LevelHeight[WorldLevel[A].Type] * LevelFrame[WorldLevel[A].Type]);
            }
        }
    }

//    If WorldEditor = True Then
//        For A = 1 To numEffects
//            With Effect(A)
//                If vScreenCollision(Z, .Location) Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffectMask(.Type), 0, .Frame * EffectHeight(.Type), vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffect(.Type), 0, .Frame * EffectHeight(.Type), vbSrcPaint
//                End If
//            End With
//        Next A
//        For A = 1 To numWorldMusic
//            With WorldMusic(A).Location
//                BitBlt myBackBuffer, vScreenX(Z) + .X, vScreenY(Z) + .Y, .Width, .Height, WarpMask(1).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, vScreenX(Z) + .X, vScreenY(Z) + .Y, .Width, .Height, Warp(1).hdc, 0, 0, vbSrcPaint
//                SuperPrint Str(WorldMusic(A).Type), 1, Int(.X + 2 + vScreenX(Z)), Int(.Y + 2 + vScreenY(Z))
//            End With
//        Next A
//        If EditorCursor.Mode = 7 Then
//            With EditorCursor.Tile
//                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXTile(.Type), 0, TileHeight(.Type) * TileFrame(.Type), vbSrcCopy
//            End With
//        End If
//        If EditorCursor.Mode = 8 Then
//            With EditorCursor.Scene
//                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXSceneMask(.Type), 0, SceneHeight(.Type) * SceneFrame(.Type), vbSrcAnd
//                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXScene(.Type), 0, SceneHeight(.Type) * SceneFrame(.Type), vbSrcPaint
//            End With
//        End If
//        If EditorCursor.Mode = 9 Then
//            With EditorCursor.WorldLevel
//                If .Path = True Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXLevelMask(0), 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXLevel(0), 0, 0, vbSrcPaint
//                End If
//                If .Path2 = True Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 16, vScreenY(Z) + 8 + .Location.Y, 64, 32, GFXLevelMask(29), 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 16, vScreenY(Z) + 8 + .Location.Y, 64, 32, GFXLevel(29), 0, 0, vbSrcPaint
//                End If
//                If GFXLevelBig(.Type) = True Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X - (GFXLevelWidth(.Type) - 32) / 2, vScreenY(Z) + .Location.Y - GFXLevelHeight(.Type) + 32, GFXLevelWidth(.Type), GFXLevelHeight(.Type), GFXLevelMask(.Type), 0, 32 * LevelFrame(.Type), vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X - (GFXLevelWidth(.Type) - 32) / 2, vScreenY(Z) + .Location.Y - GFXLevelHeight(.Type) + 32, GFXLevelWidth(.Type), GFXLevelHeight(.Type), GFXLevel(.Type), 0, 32 * LevelFrame(.Type), vbSrcPaint
//                Else
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXLevelMask(.Type), 0, 32 * LevelFrame(.Type), vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXLevel(.Type), 0, 32 * LevelFrame(.Type), vbSrcPaint
//                End If
//            End With
//        End If
//        If EditorCursor.Mode = 10 Then
//            With EditorCursor.WorldPath
//                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXPathMask(.Type), 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXPath(.Type), 0, 0, vbSrcPaint
//            End With
//        End If
//        With EditorCursor
//                If .Mode = 6 Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 2, vScreenY(Z) + .Location.Y, 22, 30, ECursorMask(3).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 2, vScreenY(Z) + .Location.Y, 22, 30, ECursor(3).hdc, 0, 0, vbSrcPaint
//                Else
//                    If .Mode = 11 Then

//                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, WarpMask(1).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, Warp(1).hdc, 0, 0, vbSrcPaint
//                SuperPrint Str(.WorldMusic.Type), 1, Int(.Location.X + 2 + vScreenX(Z)), Int(.Location.Y + 2 + vScreenY(Z))

//                        'BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, 2, Split(1).hdc, 0, 0, vbSrcCopy
//                        'BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .Location.Height - 2, .Location.Width, 2, Split(1).hdc, 0, 0, vbSrcCopy
//                        'BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, 2, .Location.Height, Split(1).hdc, 0, 0, vbSrcCopy
//                        'BitBlt myBackBuffer, vScreenX(Z) + .Location.X + .Location.Width - 2, vScreenY(Z) + .Location.Y, 2, .Location.Height, Split(1).hdc, 0, 0, vbSrcCopy
//                    End If
//                    BitBlt myBackBuffer, .X, .Y + 8, 32, 32, ECursorMask(2).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, .X, .Y + 8, 32, 32, ECursor(2).hdc, 0, 0, vbSrcPaint
//                End If
//        End With
//        'BitBlt frmLevelWindow.vScreen(Z).hdc, 0, 0, frmLevelWindow.vScreen(Z).ScaleWidth, frmLevelWindow.vScreen(Z).ScaleHeight, myBackBuffer, 0, 0, vbSrcCopy
//        StretchBlt frmLevelWindow.vScreen(Z).hdc, 0, 0, frmLevelWindow.vScreen(Z).ScaleWidth, frmLevelWindow.vScreen(Z).ScaleHeight, myBackBuffer, 0, 0, 800, 600, vbSrcCopy
//    Else
    { // NOT AN EDITOR!!!
        if(WorldPlayer[1].Type == 0)
            WorldPlayer[1].Type = 1;
        if(Player[1].Character == 1)
            WorldPlayer[1].Type = 1;
        if(Player[1].Character == 2)
            WorldPlayer[1].Type = 2;
        if(Player[1].Character == 3)
            WorldPlayer[1].Type = 3;
        if(Player[1].Character == 4)
            WorldPlayer[1].Type = 4;
        if(Player[1].Character == 5)
            WorldPlayer[1].Type = 5;
        if(WorldPlayer[1].Type == 3)
            WPHeight = 44;
        else if(WorldPlayer[1].Type == 4)
            WPHeight = 40;
        else
            WPHeight = 32;
//        frmMain.renderTexture(vScreenX[Z] + WorldPlayer[1].Location.X, vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayerMask[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);
//        frmMain.renderTexture(vScreenX[Z] + WorldPlayer[1].Location.X, vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayer[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);
        frmMain.renderTexture(vScreenX[Z] + WorldPlayer[1].Location.X,
                              vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight,
                              WorldPlayer[1].Location.Width, WPHeight,
                              GFXPlayerBMP[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);

//        frmMain.renderTexture(0, 0, 800, 130, Interface[4], 0, 0);

        frmMain.renderTexture(0, 0, 800, 130, Interface[4], 0, 0);
        frmMain.renderTexture(0, 534, 800, 66, Interface[4], 0, 534);
        frmMain.renderTexture(0, 130, 66, 404, Interface[4], 0, 130);
        frmMain.renderTexture(734, 130, 66, 404, Interface[4], 734, 130);
        for(A = 1; A <= numPlayers; A++)
        {
            Player[A].Direction = -1;
            Player[A].Location.SpeedY = 0;
            Player[A].Location.SpeedX = -1;
            Player[A].Controls.Left = false;
            Player[A].Controls.Right = false;
            if(Player[A].Duck == true)
            {
                UnDuck(A);
            }
            PlayerFrame(A);
            if(Player[A].Mount == 3)
            {
                if(Player[A].MountType == 0)
                    Player[A].MountType = 1;
                B = Player[A].MountType;
                // Yoshi's Body
                frmMain.renderTexture(32 + (48 * A) + Player[A].YoshiBX, 124 - Player[A].Location.Height + Player[A].YoshiBY,
                                      32, 32, GFXYoshiBBMP[B], 0, 32 * Player[A].YoshiBFrame, c, c, c);

                // Yoshi's Head
                frmMain.renderTexture(32 + (48 * A) + Player[A].YoshiTX,
                                      124 - Player[A].Location.Height + Player[A].YoshiTY,
                                      32, 32, GFXYoshiTBMP[B], 0, 32 * Player[A].YoshiTFrame, c, c, c);
            }
            if(Player[A].Character == 1)
            {
                if(Player[A].Mount == 0 || Player[A].Mount == 3)
                {
                    frmMain.renderTexture(32 + (48 * A) + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          124 - Player[A].Location.Height + MarioFrameY[(Player[A].State * 100) +
                                            (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY,
                                          99, 99, GFXMarioBMP[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction),
                                          pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                }
                else if(Player[A].Mount == 1)
                {
                    frmMain.renderTexture(32 + (48 * A) + MarioFrameX[(Player[A].State * 100) +
                                            (Player[A].Frame * Player[A].Direction)],
                                          124 - Player[A].Location.Height + MarioFrameY[(Player[A].State * 100) +
                                            (Player[A].Frame * Player[A].Direction)],
                                          99, Player[A].Location.Height - 26,
                                          GFXMarioBMP[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction),
                                          pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);

                    frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16, 124 - 30, 32, 32,
                                          Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;

                        frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16 + 20, 124 - 30 - 10, 32, 32,
                                              YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame);
                    }
                }
            }
            else if(Player[A].Character == 2)
            {
                if(Player[A].Mount == 0 || Player[A].Mount == 3)
                {
                    frmMain.renderTexture(32 + (48 * A) + LuigiFrameX[(Player[A].State * 100) +
                                            (Player[A].Frame * Player[A].Direction)],
                                          124 - Player[A].Location.Height + LuigiFrameY[(Player[A].State * 100) +
                                            (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY, 99, 99,
                                          GFXLuigiBMP[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction),
                                          pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                }
                else if(Player[A].Mount == 1)
                {
                    frmMain.renderTexture(32 + (48 * A) + LuigiFrameX[(Player[A].State * 100) +
                                            (Player[A].Frame * Player[A].Direction)],
                                          124 - Player[A].Location.Height +
                                            LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          99, Player[A].Location.Height - 24,
                                          GFXLuigiBMP[Player[A].State],
                                          pfrX(100 + Player[A].Frame * Player[A].Direction),
                                          pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);

                    frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16, 124 - 30, 32, 32, Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;

                        frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16 + 20, 124 - 30 - 10, 32, 32, YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, c, c, c);
                    }
                }
            }
            else if(Player[A].Character == 3)
            {
                if(Player[A].Mount == 0 || Player[A].Mount == 3)
                {
                    frmMain.renderTexture(32 + (48 * A) + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 124 - Player[A].Location.Height + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY, 99, 99, GFXPeachBMP[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                }
                else if(Player[A].Mount == 1)
                {
                    frmMain.renderTexture(32 + (48 * A) + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 124 - Player[A].Location.Height + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, Player[A].Location.Height - 24, GFXPeachBMP[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);

                    frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16, 124 - 30, 32, 32, Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;

                        frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16 + 20, 124 - 30 - 10, 32, 32, YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, c, c, c);
                    }
                }
            }
            else if(Player[A].Character == 4)
            {
                if(Player[A].Mount == 0 || Player[A].Mount == 3)
                {
                    frmMain.renderTexture(32 + (48 * A) + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 124 - Player[A].Location.Height + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY, 99, 99, GFXToadBMP[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                }
                else if(Player[A].Mount == 1)
                {
                    if(Player[A].State == 1)
                    {
                        frmMain.renderTexture(32 + (48 * A) + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 6 + 124 - Player[A].Location.Height + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, Player[A].Location.Height - 24, GFXToadBMP[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                    }
                    else
                    {
                        frmMain.renderTexture(32 + (48 * A) + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 124 - Player[A].Location.Height + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, Player[A].Location.Height - 24, GFXToadBMP[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                    }
                    frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16, 124 - 30, 32, 32, Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;
                        frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16 + 20, 124 - 30 - 10, 32, 32, YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, c, c, c);
                    }
                }
            }
            else if(Player[A].Character == 5)
            {
                frmMain.renderTexture(32 + (48 * A) + LinkFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 124 - Player[A].Location.Height + LinkFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)], 99, 99, GFXLinkBMP[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
            }
        }
        A = numPlayers + 1;
        // Print lives on the screen
        frmMain.renderTexture(32 + (48 * A), 126 - Interface[3].h, Interface[3].w, Interface[3].h, Interface[3], 0, 0);
        frmMain.renderTexture(32 + (48 * A) + 40, 128 - Interface[3].h, Interface[1].w, Interface[1].h, Interface[1], 0, 0);

        SuperPrint(std::to_string(int(Lives)), 1, 32 + (48 * A) + 62, 112);
        // Print coins on the screen
        if(Player[1].Character == 5)
        {
            frmMain.renderTexture(32 + (48 * A) + 16, 88, Interface[2].w, Interface[2].h, Interface[6], 0, 0);
        }
        else
        {
            frmMain.renderTexture(32 + (48 * A) + 16, 88, Interface[2].w, Interface[2].h, Interface[2], 0, 0);
        }
        frmMain.renderTexture(32 + (48 * A) + 40, 90, Interface[1].w, Interface[1].h, Interface[1], 0, 0);

        SuperPrint(std::to_string(Coins), 1, 32 + (48 * A) + 62, 90);
        // Print stars on the screen
        if(numStars > 0)
        {
            frmMain.renderTexture(32 + (48 * A) + 16, 66, Interface[5].w, Interface[5].h, Interface[5], 0, 0);
            frmMain.renderTexture(32 + (48 * A) + 40, 68, Interface[1].w, Interface[1].h, Interface[1], 0, 0);
            SuperPrint(std::to_string(numStars), 1, 32 + (48 * A) + 62, 68);
        }
        // Print the level's name
        if(WorldPlayer[1].LevelName != "")
        {
            SuperPrint(WorldPlayer[1].LevelName, 2, 32 + (48 * A) + 116, 109);
        }
        if(GamePaused == true)
        {
            frmMain.renderRect(210, 200, 380, 200, 0.f, 0.f, 0.f);
            if(Cheater == false)
            {
                SuperPrint("CONTINUE", 3, 272, 257);
                SuperPrint("SAVE & CONTINUE", 3, 272, 292);
                SuperPrint("SAVE & QUIT", 3, 272, 327);
                frmMain.renderTexture(252, 257 + (MenuCursor * 35), 16, 16, MCursor[0], 0, 0);
            }
            else
            {
                SuperPrint("CONTINUE", 3, 272 + 56, 275);
                SuperPrint("QUIT", 3, 272 + 56, 310);
                frmMain.renderTexture(252 + 56, 275 + (MenuCursor * 35), 16, 16, MCursor[0], 0, 0);
            }
        }
        if(PrintFPS > 0)
        {
            SuperPrint(std::to_string(int(PrintFPS)), 1, 8, 8, 0.f, 1.f, 0.f);
        }

        frmMain.repaint();
    }

    if(TakeScreen)
        ScreenShot();
}
