#include "globals.h"
#include "graphics.h"
#include "collision.h"
#include "game_main.h"
#include "gfx.h"
#include "player.h"
#include "npc.h"
#include "editor.h"
#include "sound.h"
#include "change_res.h"
#include <fmt_format_ne.h>
#include <algorithm>

#include "pseudo_vb.h"

void SpecialFrames();

// draws GFX to screen when on the world map/world map editor
void UpdateGraphics2()
{
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
    Location_t tempLocation;
    Z = 1;

    vScreen[Z].Left = 0;
    vScreen[Z].Top = 0;
    vScreen[Z].Width = ScreenW;
    vScreen[Z].Height = ScreenH;
    SpecialFrames();
    SceneFrame2[1] = SceneFrame2[1] + 1;
    if(SceneFrame2[1] >= 12)
    {
        SceneFrame2[1] = 0;
        SceneFrame[1] = SceneFrame[1] + 1;
        if(SceneFrame[1] >= 4)
            SceneFrame[1] = 0;
        SceneFrame[4] = SceneFrame[1];
        SceneFrame[5] = SceneFrame[1];
        SceneFrame[6] = SceneFrame[1];
        SceneFrame[9] = SceneFrame[1];
        SceneFrame[10] = SceneFrame[1];
        SceneFrame[12] = SceneFrame[1];
        SceneFrame[51] = SceneFrame[1];
        SceneFrame[52] = SceneFrame[1];
        SceneFrame[53] = SceneFrame[1];
        SceneFrame[54] = SceneFrame[1];
        SceneFrame[55] = SceneFrame[1];
    }
    SceneFrame2[27] = SceneFrame2[27] + 1;
    if(SceneFrame2[27] >= 8)
    {
        SceneFrame2[27] = 0;
        SceneFrame[27] = SceneFrame[27] + 1;
        if(SceneFrame[27] >= 12)
            SceneFrame[27] = 0;
        SceneFrame[28] = SceneFrame[27];
        SceneFrame[29] = SceneFrame[27];
        SceneFrame[30] = SceneFrame[27];
    }
    SceneFrame2[33] = SceneFrame2[33] + 1;
    if(SceneFrame2[33] >= 4)
    {
        SceneFrame2[33] = 0;
        SceneFrame[33] = SceneFrame[27] + 1;
        if(SceneFrame[33] >= 14)
            SceneFrame[33] = 0;
        SceneFrame[34] = SceneFrame[33];
    }
    SceneFrame2[62] = SceneFrame2[62] + 1;
    if(SceneFrame2[62] >= 8)
    {
        SceneFrame2[62] = 0;
        SceneFrame[62] = SceneFrame[62] + 1;
        if(SceneFrame[62] >= 8)
            SceneFrame[62] = 0;
        SceneFrame[63] = SceneFrame[62];
    }
    LevelFrame2[2] = LevelFrame2[2] + 1;
    if(LevelFrame2[2] >= 6)
    {
        LevelFrame2[2] = 0;
        LevelFrame[2] = LevelFrame[2] + 1;
        if(LevelFrame[2] >= 6)
            LevelFrame[2] = 0;
        LevelFrame[9] = LevelFrame[2];
        LevelFrame[13] = LevelFrame[2];
        LevelFrame[14] = LevelFrame[2];
        LevelFrame[15] = LevelFrame[2];
        LevelFrame[31] = LevelFrame[2];
        LevelFrame[32] = LevelFrame[2];
    }
    LevelFrame2[8] = LevelFrame2[8] + 1;
    if(LevelFrame2[8] >= 12)
    {
        LevelFrame2[8] = 0;
        LevelFrame[8] = LevelFrame[8] + 1;
        if(LevelFrame[8] >= 4)
            LevelFrame[8] = 0;
    }
    LevelFrame2[12] = LevelFrame2[12] + 1;
    if(LevelFrame2[12] >= 8)
    {
        LevelFrame2[12] = 0;
        LevelFrame[12] = LevelFrame[12] + 1;
        if(LevelFrame[12] >= 2)
            LevelFrame[12] = 0;
    }
    LevelFrame2[25] = LevelFrame2[25] + 1;
    if(LevelFrame2[25] >= 8)
    {
        LevelFrame2[25] = 0;
        LevelFrame[25] = LevelFrame[25] + 1;
        if(LevelFrame[25] >= 4)
            LevelFrame[25] = 0;
        LevelFrame[26] = LevelFrame[25];
    }
    TileFrame2[14] = TileFrame2[14] + 1;
    if(TileFrame2[14] >= 14)
    {
        TileFrame2[14] = 0;
        TileFrame[14] = TileFrame[14] + 1;
        if(TileFrame[14] >= 4)
            TileFrame[14] = 0;
        TileFrame[27] = TileFrame[14];
        TileFrame[241] = TileFrame[14];
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
                                      GFXPathBMP[WorldPath[A].Type], 0, 0);
            }
        }
        for(A = 1; A <= numWorldLevels; A++)
        {
            if(vScreenCollision2(1, WorldLevel[A].Location) == true && WorldLevel[A].Active == true)
            {
                if(WorldLevel[A].Path == true)
                {
                    frmMain.renderTexture(vScreenX[Z] + WorldLevel[A].Location.X,
                                          vScreenY[Z] + WorldLevel[A].Location.Y,
                                          WorldLevel[A].Location.Width,
                                          WorldLevel[A].Location.Height,
                                          GFXLevelBMP[0], 0, 0);
                }
                if(WorldLevel[A].Path2 == true)
                {
                    frmMain.renderTexture(vScreenX[Z] + WorldLevel[A].Location.X - 16,
                                          vScreenY[Z] + 8 + WorldLevel[A].Location.Y,
                                          64, 32,
                                          GFXLevelBMP[29], 0, 0);
                }
                if(GFXLevelBig[WorldLevel[A].Type] == true)
                {
                    frmMain.renderTexture(vScreenX[Z] + WorldLevel[A].Location.X - (GFXLevelWidth[WorldLevel[A].Type] - 32) / 2.0,
                                          vScreenY[Z] + WorldLevel[A].Location.Y - GFXLevelHeight[WorldLevel[A].Type] + 32,
                                          GFXLevelWidth[WorldLevel[A].Type], GFXLevelHeight[WorldLevel[A].Type],
                                          GFXLevelBMP[WorldLevel[A].Type], 0, 32 * LevelFrame[WorldLevel[A].Type]);
                }
                else
                {
                    frmMain.renderTexture(vScreenX[Z] + WorldLevel[A].Location.X,
                                          vScreenY[Z] + WorldLevel[A].Location.Y,
                                          WorldLevel[A].Location.Width, WorldLevel[A].Location.Height,
                                          GFXLevelBMP[WorldLevel[A].Type], 0, 32 * LevelFrame[WorldLevel[A].Type]);
                }
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
//                BitBlt myBackBuffer, vScreenX(Z) + .X, vScreenY(Z) + .Y, .Width, .Height, GFX.WarpMask(1).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, vScreenX(Z) + .X, vScreenY(Z) + .Y, .Width, .Height, GFX.Warp(1).hdc, 0, 0, vbSrcPaint
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
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 2, vScreenY(Z) + .Location.Y, 22, 30, GFX.ECursorMask(3).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 2, vScreenY(Z) + .Location.Y, 22, 30, GFX.ECursor(3).hdc, 0, 0, vbSrcPaint
//                Else
//                    If .Mode = 11 Then

//                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFX.WarpMask(1).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFX.Warp(1).hdc, 0, 0, vbSrcPaint
//                SuperPrint Str(.WorldMusic.Type), 1, Int(.Location.X + 2 + vScreenX(Z)), Int(.Location.Y + 2 + vScreenY(Z))

//                        'BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, 2, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                        'BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .Location.Height - 2, .Location.Width, 2, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                        'BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, 2, .Location.Height, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                        'BitBlt myBackBuffer, vScreenX(Z) + .Location.X + .Location.Width - 2, vScreenY(Z) + .Location.Y, 2, .Location.Height, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                    End If
//                    BitBlt myBackBuffer, .X, .Y + 8, 32, 32, GFX.ECursorMask(2).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, .X, .Y + 8, 32, 32, GFX.ECursor(2).hdc, 0, 0, vbSrcPaint
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

//        frmMain.renderTexture(0, 0, 800, 130, GFX.Interface[4], 0, 0);
        frmMain.renderTexture(0, 0, 800, 130, GFX.Interface[4], 0, 0);
        frmMain.renderTexture(0, 534, 800, 66, GFX.Interface[4], 0, 534);
        frmMain.renderTexture(0, 130, 66, 404, GFX.Interface[4], 0, 130);
        frmMain.renderTexture(734, 130, 66, 404, GFX.Interface[4], 734, 130);
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
                                          GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;

                        frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16 + 20, 124 - 30 - 10, 32, 32,
                                              GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame);
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

                    frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16, 124 - 30, 32, 32, GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;

                        frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, c, c, c);
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

                    frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16, 124 - 30, 32, 32, GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;

                        frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, c, c, c);
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
                    frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16, 124 - 30, 32, 32, GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount = Player[A].YoshiWingsFrameCount + 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;
                        frmMain.renderTexture(32 + (48 * A) + Player[A].Location.Width / 2.0 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, c, c, c);
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
        frmMain.renderTexture(32 + (48 * A), 126 - GFX.Interface[3].h, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
        frmMain.renderTexture(32 + (48 * A) + 40, 128 - GFX.Interface[3].h, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

        SuperPrint(std::to_string(int(Lives)), 1, 32 + (48 * A) + 62, 112);
        // Print coins on the screen
        if(Player[1].Character == 5)
        {
            frmMain.renderTexture(32 + (48 * A) + 16, 88, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[6], 0, 0);
        }
        else
        {
            frmMain.renderTexture(32 + (48 * A) + 16, 88, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[2], 0, 0);
        }
        frmMain.renderTexture(32 + (48 * A) + 40, 90, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

        SuperPrint(std::to_string(Coins), 1, 32 + (48 * A) + 62, 90);
        // Print stars on the screen
        if(numStars > 0)
        {
            frmMain.renderTexture(32 + (48 * A) + 16, 66, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
            frmMain.renderTexture(32 + (48 * A) + 40, 68, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
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
                frmMain.renderTexture(252, 257 + (MenuCursor * 35), 16, 16, GFX.MCursor[0], 0, 0);
            }
            else
            {
                SuperPrint("CONTINUE", 3, 272 + 56, 275);
                SuperPrint("QUIT", 3, 272 + 56, 310);
                frmMain.renderTexture(252 + 56, 275 + (MenuCursor * 35), 16, 16, GFX.MCursor[0], 0, 0);
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

// This draws the graphic to the screen when in a level/game menu/outro/level editor
void UpdateGraphics()
{
//    On Error Resume Next
    float c = ShadowMode ? 0.f : 1.f;
//    Dim A As Integer
    int A = 0;
//    Dim timeStr As String
    std::string timeStr;
//    Dim Z As Integer
    int Z = 0;
//    Dim numScreens As Integer
    int numScreens = 0;

//'frame skip code
//    cycleCount = cycleCount + 1
    cycleCount += 1;
//    If FrameSkip = True And TakeScreen = False Then
    if(FrameSkip And !TakeScreen)
    {
        if(SDL_GetTicks() + floor(1000 * (1 - (cycleCount / 63.0))) > GoalTime) // Don't draw this frame
        {
            numScreens = 1;
            if(LevelEditor == false)
            {
                if(ScreenType == 1)
                    numScreens = 2;
                if(ScreenType == 4)
                    numScreens = 2;
                if(ScreenType == 5)
                {
                    DynamicScreen();
                    if(vScreen[2].Visible == true)
                        numScreens = 2;
                    else
                        numScreens = 1;
                }
                if(ScreenType == 8)
                    numScreens = 1;
            }

            for(Z = 1; Z <= numScreens; Z++)
            {
                if(LevelEditor == false)
                {
                    if(ScreenType == 2 || ScreenType == 3)
                        GetvScreenAverage();
                    else if(ScreenType == 5 && vScreen[2].Visible == false)
                        GetvScreenAverage();
                    else if(ScreenType == 7)
                        GetvScreenCredits();
                    else
                    {
                        GetvScreen(Z);
                    }
                }
                for(A = 1; A <= numNPCs; A++)
                {
                    if(vScreenCollision(Z, NPC[A].Location) && NPC[A].Hidden == false)
                    {
                        if(NPC[A].Reset[Z] == true || NPC[A].Active == true)
                        {
                            if(NPC[A].Active == false)
                            {
                                NPC[A].JustActivated = Z;
//                                if(nPlay.Online == true)
//                                {
//                                    Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                    NPC[A].JustActivated = nPlay.MySlot + 1;
//                                }
                            }
                            NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                            if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                timeStr = timeStr + "2b" + std::to_string(A) + LB;
                            NPC[A].Active = true;
                        }
                        NPC[A].Reset[1] = false;
                        NPC[A].Reset[2] = false;
                    }
                    else
                    {
                        NPC[A].Reset[Z] = true;
                        if(numScreens == 1)
                            NPC[A].Reset[2] = true;
                        if(SingleCoop == 1)
                            NPC[A].Reset[2] = true;
                        else if(SingleCoop == 2)
                            NPC[A].Reset[1] = true;
                    }
                }
            }
            return;
        }
    }
    fpsCount++;

    std::string SuperText;
    std::string tempText;
    int BoxY = 0;
    bool tempBool = false;
    int B = 0;
    int B2 = 0;
    int C = 0;
    int D = 0;
    int E = 0;
    int d2 = 0;
    int e2 = 0;
    int X = 0;
    int Y = 0;
    int fBlock = 0;
    int lBlock = 0;
    Location_t tempLocation;
//    Dim S As Integer 'Level section to display
    int S = 0; // Level section to display
//    If Score > 9999990 Then Score = 9999990
    if(Score > 9999990)
        Score = 9999990;
//    If Lives > 99 Then Lives = 99
    if(Lives > 99)
        Lives = 99;
//    numScreens = 1
    numScreens = 1;
//    If TakeScreen = True Then
    if(TakeScreen) // DUMMY
    {
//        If LevelEditor = True Or MagicHand = True Then
//            frmLevelWindow.vScreen(1).AutoRedraw = True
//        Else
//            frmMain.AutoRedraw = True
//        End If
//    End If
    }

//    'Background frames
//    If FreezeNPCs = False Then
    if(!FreezeNPCs)
    {
        BackgroundFrameCount[26]++;
        if(BackgroundFrameCount[26] >= 8)
        {
            BackgroundFrame[26]++;
            if(BackgroundFrame[26] >= 8)
                BackgroundFrame[26] = 0;
            BackgroundFrameCount[26] = 0;
        }
        BackgroundFrameCount[18]++;
        if(BackgroundFrameCount[18] >= 12)
        {
            BackgroundFrame[18]++;
            if(BackgroundFrame[18] >= 4)
                BackgroundFrame[18] = 0;
            BackgroundFrame[19] = BackgroundFrame[18];
            BackgroundFrame[20] = BackgroundFrame[18];
            BackgroundFrame[161] = BackgroundFrame[18];
            BackgroundFrameCount[18] = 0;
        }
        BackgroundFrameCount[36] = BackgroundFrameCount[36] + 1;
        if(BackgroundFrameCount[36] >= 2)
        {
            BackgroundFrame[36] = BackgroundFrame[36] + 1;
            if(BackgroundFrame[36] >= 4)
                BackgroundFrame[36] = 0;
            BackgroundFrameCount[36] = 0;
        }
        BackgroundFrame[68] = BackgroundFrame[36];
        BackgroundFrameCount[65] = BackgroundFrameCount[65] + 1;
        if(BackgroundFrameCount[65] >= 8)
        {
            BackgroundFrame[65] = BackgroundFrame[65] + 1;
            if(BackgroundFrame[65] >= 4)
                BackgroundFrame[65] = 0;
            BackgroundFrameCount[65] = 0;
        }

        BackgroundFrame[66] = BackgroundFrame[65];

        BackgroundFrame[70] = BackgroundFrame[65];
        BackgroundFrame[100] = BackgroundFrame[65];

        BackgroundFrame[134] = BackgroundFrame[65];
        BackgroundFrame[135] = BackgroundFrame[65];
        BackgroundFrame[136] = BackgroundFrame[65];
        BackgroundFrame[137] = BackgroundFrame[65];
        BackgroundFrame[138] = BackgroundFrame[65];


        BackgroundFrameCount[82] = BackgroundFrameCount[82] + 1;
        if(BackgroundFrameCount[82] >= 10)
        {
            BackgroundFrame[82] = BackgroundFrame[82] + 1;
            if(BackgroundFrame[82] >= 4)
                BackgroundFrame[82] = 0;
            BackgroundFrameCount[82] = 0;
        }

        BackgroundFrameCount[170] = BackgroundFrameCount[170] + 1;
        if(BackgroundFrameCount[170] >= 8)
        {
            BackgroundFrame[170] = BackgroundFrame[170] + 1;
            if(BackgroundFrame[170] >= 4)
                BackgroundFrame[170] = 0;
            BackgroundFrame[171] = BackgroundFrame[170];
            BackgroundFrameCount[170] = 0;
        }

        BackgroundFrameCount[125] = BackgroundFrameCount[125] + 1;
        if(BackgroundFrameCount[125] >= 4)
        {
            if(BackgroundFrame[125] == 0)
                BackgroundFrame[125] = 1;
            else
                BackgroundFrame[125] = 0;
            BackgroundFrameCount[125] = 0;
        }
        SpecialFrames();
    }

//        BackgroundFrame(172) = BackgroundFrame(66)
    BackgroundFrame[172] = BackgroundFrame[66];
//    BackgroundFrameCount(158) = BackgroundFrameCount(158) + 1
    BackgroundFrameCount[158] += 1;
//    If BackgroundFrameCount(158) >= 6 Then
    if(BackgroundFrameCount[158] >= 6)
    {
        BackgroundFrameCount[158] = 0;
        BackgroundFrame[158] = BackgroundFrame[158] + 1;
        BackgroundFrame[159] = BackgroundFrame[159] + 1;
        if(BackgroundFrame[158] >= 4)
            BackgroundFrame[158] = 0;
        if(BackgroundFrame[159] >= 8)
            BackgroundFrame[159] = 0;
    }

    BackgroundFrameCount[168] = BackgroundFrameCount[168] + 1;
    if(BackgroundFrameCount[168] >= 8)
    {
        BackgroundFrame[168] = BackgroundFrame[168] + 1;
        if(BackgroundFrame[168] >= 8)
            BackgroundFrame[168] = 0;
        BackgroundFrameCount[168] = 0;
    }

    BackgroundFrameCount[173] = BackgroundFrameCount[173] + 1;
    if(BackgroundFrameCount[173] >= 8)
    {
        BackgroundFrameCount[173] = 0;
        if(BackgroundFrame[173] == 0)
            BackgroundFrame[173] = 1;
        else
            BackgroundFrame[173] = 0;
    }

    BackgroundFrameCount[187] = BackgroundFrameCount[187] + 1;
    if(BackgroundFrameCount[187] >= 6)
    {
        BackgroundFrame[187] = BackgroundFrame[187] + 1;
        if(BackgroundFrame[187] >= 4)
            BackgroundFrame[187] = 0;
        BackgroundFrame[188] = BackgroundFrame[187];
        BackgroundFrame[189] = BackgroundFrame[187];
        BackgroundFrame[190] = BackgroundFrame[187];
        BackgroundFrameCount[187] = 0;
    }


//    If LevelEditor = False Then  'Sets up the screens if not in level editor
    { // NOT AN EDITOR
        if(ScreenType == 1)
            numScreens = 2;
        if(ScreenType == 4)
            numScreens = 2;
        if(ScreenType == 5)
        {
            DynamicScreen();
            if(vScreen[2].Visible)
                numScreens = 2;
            else
                numScreens = 1;
        }
        if(ScreenType == 8)
            numScreens = 1;
    }

    if(ClearBuffer)
    {
        ClearBuffer = False;
        frmMain.clearBuffer();
    }

//    If SingleCoop = 2 Then numScreens = 2
    if(SingleCoop == 2)
        numScreens = 2;
//    For Z = 1 To numScreens
    For(Z, 1, numScreens)
    {
//        If SingleCoop = 2 Then Z = 2
        if(SingleCoop == 2)
            Z = 2;
//        If LevelEditor = True Then
//            S = curSection
//        ElseIf nPlay.Online = True Then
//            S = Player(nPlay.MySlot + 1).Section
//        Else
        {
//            S = Player(Z).Section
            S = Player[Z].Section;
//        End If
        }

//        If GameOutro = True Then ScreenType = 7
        if(GameOutro)
            ScreenType = 7;
//        If LevelEditor = False Then
        {
//            If ScreenType = 2 Or ScreenType = 3 Then
            if(ScreenType == 2 Or ScreenType == 3)
//                GetvScreenAverage
                GetvScreenAverage();
//            ElseIf ScreenType = 5 And vScreen(2).Visible = False Then
            else if(ScreenType == 5 And !vScreen[2].Visible)
//                GetvScreenAverage
                GetvScreenAverage();
//            ElseIf ScreenType = 7 Then
            else if(ScreenType == 7)
//                GetvScreenCredits
                GetvScreenCredits();
//            Else
            else
//                GetvScreen Z
                GetvScreen(Z);
//            End If
//            If Background2(S) = 0 Then BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness
            if(Background2[S] == 0)
                frmMain.clearBuffer();
        }
//        Else
//            If Background2(S) = 0 Then BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness
//        End If

        if(qScreen == true)
        {
            if(vScreenX[1] < qScreenX[1] - 2)
                qScreenX[1] = qScreenX[1] - 2;
            else if(vScreenX[1] > qScreenX[1] + 2)
                qScreenX[1] = qScreenX[1] + 2;
            if(vScreenY[1] < qScreenY[1] - 2)
                qScreenY[1] = qScreenY[1] - 2;
            else if(vScreenY[1] > qScreenY[1] + 2)
                qScreenY[1] = qScreenY[1] + 2;
            if(qScreenX[1] < vScreenX[1] + 5 && qScreenX[1] > vScreenX[1] - 5 &&
               qScreenY[1] < vScreenY[1] + 5 && qScreenY[1] > vScreenY[1] - 5)
                qScreen = false;
            vScreenX[1] = qScreenX[1];
            vScreenY[1] = qScreenY[1];
        }

        // noturningback
        if(!LevelEditor)
        {
            if(NoTurnBack[Player[Z].Section] == true)
            {
                A = Z;
                if(numScreens > 1)
                {
                    if(Player[1].Section == Player[2].Section)
                    {
                        if(Z == 1)
                            GetvScreen(2);
                        if(-vScreenX[1] < -vScreenX[2])
                            A = 1;
                        else
                            A = 2;
                    }
                }
                if(-vScreenX[A] > level[S].X)
                {
                    LevelChop[S] = LevelChop[S] + -vScreenX[A] - level[S].X;
                    level[S].X = -vScreenX[A];
                }
            }
        }

        if(numScreens > 1) // To separate drawing of screens
            frmMain.setViewport(vScreen[Z].Left, vScreen[Z].Top, vScreen[Z].Width, vScreen[Z].Height);

        DrawBackground(S, Z);


//            If LevelEditor = True Then
//                If BlockFlash <= 30 Then
//                    With tempLocation 'Black out the level edges
//                        .X = level(curSection).X
//                        .Y = level(curSection).Y
//                        .Width = level(curSection).Width
//                        .Height = level(curSection).Height
//                        BitBlt myBackBuffer, 0, 0, vScreenX(Z) + level(curSection).X, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, 0, 0, vScreenX(Z) + level(curSection).Width, vScreenY(Z) + level(curSection).Y, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        If -vScreenX(Z) < level(curSection).Width Then
//                            BitBlt myBackBuffer, vScreenX(Z) + level(curSection).Width, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        End If
//                        If -vScreenY(Z) < level(curSection).Height Then
//                            BitBlt myBackBuffer, 0, vScreenY(Z) + level(curSection).Height, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        End If
//                    End With
//                End If
//            End If


//        If GameMenu = True Then
        if(GameMenu)
        {
            // (Commented out in original code :thinking:)
            // Curtain
//            frmMain.renderTexture(0, 0, GFX.MenuGFX[1]);
            // Game logo
//            frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 70, GFX.MenuGFX[2]);
//        ElseIf LevelEditor = False Then
        } else { // it's NO editor, obviously
//            If numPlayers > 2 And nPlay.Online = False Then
            if(numPlayers > 2)
            {
//                For A = 1 To numPlayers
                For(A, 1, numPlayers)
                {
//                    With Player(A)
                    Player_t &p = Player[A];
//                        If vScreenCollision(Z, .Location) = False And LevelMacro = 0 And .Location.Y < level(.Section).Height And .Location.Y + .Location.Height > level(.Section).Y And .TimeToLive = 0 And .Dead = False Then
                    if(!vScreenCollision(Z, p.Location) And LevelMacro == 0 And
                        p.Location.Y < level[p.Section].Height And
                        p.Location.Y + p.Location.Height > level[p.Section].Y And
                        p.TimeToLive == 0 And !p.Dead)
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Section == Player[A].Section && vScreenCollision(Z, Player[B].Location) == true)
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
                            for(B = 1; B <= numPlayers; B++)
                            {
                                if(Player[B].Dead == false && Player[B].TimeToLive == 0 && Player[B].Section == Player[A].Section)
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
                        Player[A].Location.SpeedY = std::rand() % 12 - 6;
                        Player[A].CanJump = true;
                    }
//                    End With
//                Next A
                }
//            End If
            }
//        End If
        }

//        If LevelEditor = True Or MagicHand = True Then
//            For A = 1 To numBackground 'First backgrounds
//                With Background(A)
//                    If .Type = 11 Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, 0, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, 0, vbSrcPaint
//                        End If
//                    ElseIf .Type = 12 Or .Type = 60 Or .Type = 61 Or .Type = 75 Or .Type = 76 Or .Type = 77 Or .Type = 78 Or .Type = 79 Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, 0, vbSrcCopy
//                        End If
//                    End If
//                End With
//            Next A
//        Else
        {// NOT AN EDITOR
//            For A = 1 To MidBackground - 1 'First backgrounds
            For(A, 1, MidBackground - 1)  // First backgrounds
            {
//                if(BackgroundHasNoMask[Background[A].Type] == false) // Useless code
//                {
//                    if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
//                    {
//                        frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X,
//                                              vScreenY[Z] + Background[A].Location.Y,
//                                              BackgroundWidth[Background[A].Type],
//                                              BackgroundHeight[Background[A].Type],
//                                              GFXBackgroundBMP[Background[A].Type], 0,
//                                              BackgroundHeight[Background[A].Type] *
//                                              BackgroundFrame[Background[A].Type]);
//                    }
//                }
//                else
//                {
                if(vScreenCollision(Z, Background[A].Location) && !Background[A].Hidden)
                {
                    frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X,
                                          vScreenY[Z] + Background[A].Location.Y,
                                          GFXBackgroundWidth[Background[A].Type],
                                          BackgroundHeight[Background[A].Type],
                                          GFXBackgroundBMP[Background[A].Type], 0,
                                          BackgroundHeight[Background[A].Type] *
                                          BackgroundFrame[Background[A].Type]);
                }
//                }
            }
//        End If
        }

        tempLocation.Width = 32;
        tempLocation.Height = 32;
//        For A = 1 To sBlockNum 'Display sizable blocks
        For(A, 1, sBlockNum) // Display sizable blocks
        {
            if(BlockIsSizable[Block[sBlockArray[A]].Type] && (!(Block[sBlockArray[A]].Invis == true) || LevelEditor == true))
            {
                if(vScreenCollision(Z, Block[sBlockArray[A]].Location) && Block[sBlockArray[A]].Hidden == false)
                {
                    int bHeight = Block[sBlockArray[A]].Location.Height / 32.0;
                    for(B = 0; B < bHeight; B++)
                    {
                        int bWidth = Block[sBlockArray[A]].Location.Width / 32.0;
                        for(C = 0; C < bWidth; C++)
                        {
                            tempLocation.X = Block[sBlockArray[A]].Location.X + C * 32;
                            tempLocation.Y = Block[sBlockArray[A]].Location.Y + B * 32;
                            if(vScreenCollision(Z, tempLocation))
                            {
                                D = C;
                                E = B;
                                if(!(D == 0))
                                {
                                    if(D == (Block[sBlockArray[A]].Location.Width / 32.0) - 1)
                                        D = 2;
                                    else
                                    {
                                        D = 1;
                                        d2 = int(0.5);
                                    }
                                }
                                if(!(E == 0))
                                {
                                    if(E == (Block[sBlockArray[A]].Location.Height / 32.0) - 1)
                                        E = 2;
                                    else
                                        E = 1;
                                }
                                if((D == 0 || D == 2) || (E == 0 || E == 2) || (Block[sBlockArray[A]].Type == 130 && (D == 0 || D == 2) && E == 1))
                                {
                                    frmMain.renderTexture(vScreenX[Z] + Block[sBlockArray[A]].Location.X + C * 32, vScreenY[Z] + Block[sBlockArray[A]].Location.Y + B * 32, 32, 32, GFXBlockBMP[Block[sBlockArray[A]].Type], D * 32, E * 32);
                                }
                                else
                                {
                                    frmMain.renderTexture(vScreenX[Z] + Block[sBlockArray[A]].Location.X + C * 32, vScreenY[Z] + Block[sBlockArray[A]].Location.Y + B * 32, 32, 32, GFXBlockBMP[Block[sBlockArray[A]].Type], D * 32, E * 32);
                                }
                            }
                        }
                    }
                }
            }
        }

//        If LevelEditor = True Or MagicHand = True Then
//            For A = 1 To numBackground 'Second backgrounds
//                With Background(A)
//                    If Not (.Type = 11 Or .Type = 12 Or .Type = 60 Or .Type = 61 Or .Type = 75 Or .Type = 76 Or .Type = 77 Or .Type = 78 Or .Type = 79) Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            If BackgroundHasNoMask(.Type) = False Then
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcAnd
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcPaint
//                            Else
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcCopy
//                            End If
//                        End If
//                    End If
//                End With
//            Next A
//        Else
        { // NOT AN EDITOR
            for(A = MidBackground; A <= LastBackground; A++) // Second backgrounds
            {
                if(vScreenCollision(Z, Background[A].Location) && Background[A].Hidden == false)
                {
                    if(BackgroundHasNoMask[Background[A].Type] == false)
                    {
                        frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X, vScreenY[Z] + Background[A].Location.Y, BackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackgroundBMP[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X, vScreenY[Z] + Background[A].Location.Y, BackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackgroundBMP[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                    }
                }
            }
        }

//        For A = numBackground + 1 To numBackground + numLocked 'Locked doors
        For(A, numBackground + 1, numBackground + numLocked)
        {
            if(vScreenCollision(Z, Background[A].Location) && (Background[A].Type == 98 || Background[A].Type == 160) && Background[A].Hidden == false)
            {
                if(BackgroundHasNoMask[Background[A].Type] == false)
                {
                    frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X, vScreenY[Z] + Background[A].Location.Y, BackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackgroundBMP[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
                else
                {
                    frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X, vScreenY[Z] + Background[A].Location.Y, BackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackground[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
        }

//        For A = 1 To numNPCs 'Display NPCs that should be behind blocks
        For(A, 1, numNPCs) // Display NPCs that should be behind blocks
        {
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(((NPC[A].Effect == 208 || NPCIsAVine[NPC[A].Type] == true ||
                 NPC[A].Type == 209 || NPC[A].Type == 159 || NPC[A].Type == 245 ||
                 NPC[A].Type == 8 || NPC[A].Type == 93 || NPC[A].Type == 74 ||
                 NPC[A].Type == 256 || NPC[A].Type == 257 || NPC[A].Type == 51 ||
                 NPC[A].Type == 52 || NPC[A].Effect == 1 || NPC[A].Effect == 3 ||
                 NPC[A].Effect == 4 || (NPC[A].Type == 45 && NPC[A].Special == 0.0)) &&
                 (NPC[A].standingOnPlayer == 0 && (!NPC[A].Generator || LevelEditor == true))) ||
                 NPC[A].Type == 179 || NPC[A].Type == 270)
            {
                if(!(NPC[A].Effect == 2) && (NPC[A].Generator == false || LevelEditor == true))
                {
                    if(vScreenCollision(Z, NPC[A].Location) && NPC[A].Hidden == false)
                    {
                        if(NPC[A].Active == true)
                        {
                            if(NPC[A].Type == 8 || NPC[A].Type == 74 || NPC[A].Type == 93 || NPC[A].Type == 245 || NPC[A].Type == 256 || NPC[A].Type == 270)
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type], cn, cn, cn);
                            }
                            else if(NPC[A].Type == 51 || NPC[A].Type == 257)
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type] + NPCHeight[NPC[A].Type] - NPC[A].Location.Height, cn, cn, cn);
                            }
                            else if(NPC[A].Type == 52)
                            {
                                if(NPC[A].Direction == -1)
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type]);
                                }
                                else
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], NPCWidth[NPC[A].Type] - NPC[A].Location.Width, NPC[A].Frame * NPCHeight[NPC[A].Type], cn, cn, cn);
                                }
                            }
                            else if(NPCWidthGFX[NPC[A].Type] == 0 || NPC[A].Effect == 1)
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeight[NPC[A].Type], cn ,cn ,cn);
                            }
                            else
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type] - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                            }
                        }
                        if(NPC[A].Reset[Z] == true || NPC[A].Active == true)
                        {
                            if(NPC[A].Active == false)
                            {
                                NPC[A].JustActivated = Z;
//                                if(nPlay.Online == true)
//                                {
//                                    Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                    NPC[A].JustActivated = nPlay.MySlot + 1;
//                                }
                            }
                            NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                            if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                timeStr = timeStr + "2b" + std::to_string(A) + LB;
                            NPC[A].Active = true;
                        }
                        NPC[A].Reset[1] = false;
                        NPC[A].Reset[2] = false;
                    }
                    else
                    {
                        NPC[A].Reset[Z] = true;
                        if(numScreens == 1)
                            NPC[A].Reset[2] = true;
                        if(SingleCoop == 1)
                            NPC[A].Reset[2] = true;
                        else if(SingleCoop == 2)
                            NPC[A].Reset[1] = true;
                    }
                }
            }
        }


//        For A = 1 To numPlayers 'Players behind blocks
        For(A, 1, numPlayers)
        {
            if(Player[A].Dead == false && Player[A].Immune2 == false && Player[A].TimeToLive == 0 && Player[A].Effect == 3)
            {
                float Y2 = 0;
                float X2 = 0;

                if(vScreenCollision(Z, Player[A].Location))
                {
                    // warp NPCs
                    if(Player[A].HoldingNPC > 0 && Player[A].Frame != 15)
                    {
                        if((vScreenCollision(Z, NPC[Player[A].HoldingNPC].Location) | vScreenCollision(Z, newLoc(NPC[Player[A].HoldingNPC].Location.X - (NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] - NPC[Player[A].HoldingNPC].Location.Width) / 2.0, NPC[Player[A].HoldingNPC].Location.Y, static_cast<double>(NPCWidthGFX[NPC[Player[A].HoldingNPC].Type]), static_cast<double>(NPCHeight[NPC[Player[A].HoldingNPC].Type])))) != 0 && NPC[Player[A].HoldingNPC].Hidden == false)
                        {
                            tempLocation = NPC[Player[A].HoldingNPC].Location;
                            if(NPCHeightGFX[NPC[Player[A].HoldingNPC].Type] != 0 || NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] != 0)
                            {
                                tempLocation.Height = NPCHeightGFX[NPC[Player[A].HoldingNPC].Type];
                                tempLocation.Width = NPCWidthGFX[NPC[Player[A].HoldingNPC].Type];
                                tempLocation.Y = NPC[Player[A].HoldingNPC].Location.Y + NPC[Player[A].HoldingNPC].Location.Height - NPCHeightGFX[NPC[Player[A].HoldingNPC].Type];
                                tempLocation.X = NPC[Player[A].HoldingNPC].Location.X + NPC[Player[A].HoldingNPC].Location.Width / 2.0 - NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] / 2.0;
                            }
                            else
                            {
                                tempLocation.Height = NPCHeight[NPC[Player[A].HoldingNPC].Type];
                                tempLocation.Width = NPCWidth[NPC[Player[A].HoldingNPC].Type];
                            }
                            tempLocation.X = tempLocation.X + NPCFrameOffsetX[NPC[Player[A].HoldingNPC].Type];
                            tempLocation.Y = tempLocation.Y + NPCFrameOffsetY[NPC[Player[A].HoldingNPC].Type];
                            Y2 = 0;
                            X2 = 0;
                            NPCWarpGFX(A, tempLocation, X2, Y2);
                            if(NPCHeightGFX[NPC[Player[A].HoldingNPC].Type] != 0 || NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] != 0)
                            {
                                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[Player[A].HoldingNPC].Type], X2, Y2 + NPC[Player[A].HoldingNPC].Frame * NPCHeightGFX[NPC[Player[A].HoldingNPC].Type]);
                            }
                            else
                            {
                                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[Player[A].HoldingNPC].Type], X2, Y2 + NPC[Player[A].HoldingNPC].Frame * NPCHeight[NPC[Player[A].HoldingNPC].Type]);
                            }

                        }
                    }


                    if(Player[A].Mount == 3)
                    {
                        B = Player[A].MountType;
                        // Yoshi's Body
                        tempLocation = Player[A].Location;
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X = tempLocation.X + Player[A].YoshiBX;
                        tempLocation.Y = tempLocation.Y + Player[A].YoshiBY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXYoshiB[B], X2, Y2 + 32 * Player[A].YoshiBFrame, c, c, c);
                        // Yoshi's Head
                        tempLocation = Player[A].Location;
                        tempLocation.Height = 32;
                        tempLocation.Width = 32;
                        tempLocation.X = tempLocation.X + Player[A].YoshiTX;
                        tempLocation.Y = tempLocation.Y + Player[A].YoshiTY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXYoshiT[B], X2, Y2 + 32 * Player[A].YoshiTFrame);
                    }
                    if(Player[A].Character == 1)
                    {
                        if(Player[A].Mount == 1)
                        {
                            tempLocation = Player[A].Location;
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXMario[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X = tempLocation.X + Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y = tempLocation.Y + Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot[Player[A].MountType], X2, Y2 + 32 * Player[A].MountFrame);
                        }
                        else
                        {
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 99;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXMario[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                        }
                    }
                    else if(Player[A].Character == 2)
                    {
                       if(Player[A].Mount == 1)
                       {
                            tempLocation = Player[A].Location;
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLuigi[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X = tempLocation.X + Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y = tempLocation.Y + Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot[Player[A].MountType], X2, Y2 + 32 * Player[A].MountFrame, c, c, c);
                        }
                        else
                        {
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 99;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLuigi[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2);
                        }
                    }
                    else if(Player[A].Character == 3)
                    {
                       if(Player[A].Mount == 1)
                       {
                            tempLocation = Player[A].Location;
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXPeach[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X = tempLocation.X + Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y = tempLocation.Y + Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot[Player[A].MountType], X2, Y2 + 32 * Player[A].MountFrame, c, c, c);
                        }
                        else
                        {
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 99;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXPeach[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                        }
                    }
                    else if(Player[A].Character == 4)
                    {
                       if(Player[A].Mount == 1)
                       {
                            tempLocation = Player[A].Location;
                            if(Player[A].State == 1)
                                tempLocation.Height = Player[A].Location.Height - ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            else
                                tempLocation.Height = Player[A].Location.Height - ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 26;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            if(Player[A].State == 1)
                                tempLocation.Y = tempLocation.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + 6;
                            else
                                tempLocation.Y = tempLocation.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 4;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXToad[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2);
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 32;
                            tempLocation.Width = 32;
                            tempLocation.X = tempLocation.X + Player[A].Location.Width / 2.0 - 16;
                            tempLocation.Y = tempLocation.Y + Player[A].Location.Height - 30;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot[Player[A].MountType], X2, Y2 + 32 * Player[A].MountFrame, c, c, c);
                        }
                        else
                        {
                            tempLocation = Player[A].Location;
                            tempLocation.Height = 99;
                            tempLocation.Width = 99;
                            tempLocation.X = tempLocation.X + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                            tempLocation.Y = tempLocation.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                            Y2 = 0;
                            X2 = 0;
                            PlayerWarpGFX(A, tempLocation, X2, Y2);
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXToad[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                        }
                    }
                    else if(Player[A].Character == 5)
                    {
                        if(Player[A].Frame > 5)
                            Player[A].Frame = 1;
                        tempLocation = Player[A].Location;
                        tempLocation.Height = 99;
                        tempLocation.Width = 99;
                        tempLocation.X = tempLocation.X + LinkFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                        tempLocation.Y = tempLocation.Y + LinkFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY;
                        Y2 = 0;
                        X2 = 0;
                        PlayerWarpGFX(A, tempLocation, X2, Y2);
                        frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLink[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction) + X2, pfrY(100 + Player[A].Frame * Player[A].Direction) + Y2, c, c, c);
                    }
                }
                if(Player[A].HoldingNPC > 0 && Player[A].Frame == 15)
                {
                    if((vScreenCollision(Z, NPC[Player[A].HoldingNPC].Location) | vScreenCollision(Z, newLoc(NPC[Player[A].HoldingNPC].Location.X - (NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] - NPC[Player[A].HoldingNPC].Location.Width) / 2.0, NPC[Player[A].HoldingNPC].Location.Y, static_cast<double>(NPCWidthGFX[NPC[Player[A].HoldingNPC].Type]), static_cast<double>(NPCHeight[NPC[Player[A].HoldingNPC].Type])))) != 0 && NPC[Player[A].HoldingNPC].Hidden == false && NPC[Player[A].HoldingNPC].Type != 263)
                    {
                        tempLocation = NPC[Player[A].HoldingNPC].Location;
                        if(NPCHeightGFX[NPC[Player[A].HoldingNPC].Type] != 0 || NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] != 0)
                        {
                            tempLocation.Height = NPCHeightGFX[NPC[Player[A].HoldingNPC].Type];
                            tempLocation.Width = NPCWidthGFX[NPC[Player[A].HoldingNPC].Type];
                            tempLocation.Y = NPC[Player[A].HoldingNPC].Location.Y + NPC[Player[A].HoldingNPC].Location.Height - NPCHeightGFX[NPC[Player[A].HoldingNPC].Type];
                            tempLocation.X = NPC[Player[A].HoldingNPC].Location.X + NPC[Player[A].HoldingNPC].Location.Width / 2.0 - NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] / 2.0;
                        }
                        else
                        {
                            tempLocation.Height = NPCHeight[NPC[Player[A].HoldingNPC].Type];
                            tempLocation.Width = NPCWidth[NPC[Player[A].HoldingNPC].Type];
                        }
                        tempLocation.X = tempLocation.X + NPCFrameOffsetX[NPC[Player[A].HoldingNPC].Type];
                        tempLocation.Y = tempLocation.Y + NPCFrameOffsetY[NPC[Player[A].HoldingNPC].Type];
                        Y2 = 0;
                        X2 = 0;
                        NPCWarpGFX(A, tempLocation, X2, Y2);
                        if(NPCHeightGFX[NPC[Player[A].HoldingNPC].Type] != 0 || NPCWidthGFX[NPC[Player[A].HoldingNPC].Type] != 0)
                        {
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[Player[A].HoldingNPC].Type], X2, Y2 + NPC[Player[A].HoldingNPC].Frame * NPCHeightGFX[NPC[Player[A].HoldingNPC].Type]);
                        }
                        else
                        {
                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[Player[A].HoldingNPC].Type], X2, Y2 + NPC[Player[A].HoldingNPC].Frame * NPCHeight[NPC[Player[A].HoldingNPC].Type]);
                        }

                    }
                }
            }
        }


//        If LevelEditor = True Then
//            fBlock = 1
//            lBlock = numBlock
//        Else
        { // NOT AN EDITOR!!!
//            fBlock = FirstBlock((-vScreenX(Z) / 32) - 1)
            fBlock = FirstBlock[int(-vScreenX[Z] / 32) - 1];
//            lBlock = LastBlock(((-vScreenX(Z) + vScreen(Z).Width) / 32) + 1)
            lBlock = LastBlock[int((-vScreenX[Z] + vScreen[Z].Width) / 32) + 1];
//        End If
        }


//        For A = fBlock To lBlock 'Non-Sizable Blocks
        For(A, fBlock, lBlock)
        {
            if(!BlockIsSizable[Block[A].Type] && (!(Block[A].Invis == true) || (LevelEditor == true && BlockFlash <= 30)) && !(Block[A].Type == 0) && !BlockKills[Block[A].Type])
            {
                if(vScreenCollision(Z, Block[A].Location) && Block[A].Hidden == false)
                {
                    frmMain.renderTexture(vScreenX[Z] + Block[A].Location.X, vScreenY[Z] + Block[A].Location.Y + Block[A].ShakeY3, Block[A].Location.Width, Block[A].Location.Height, GFXBlock[Block[A].Type], 0, BlockFrame[Block[A].Type] * 32);
                }
            }
        }

//'effects in back
        for(A = 1; A <= numEffects; A++)
        {
            if(Effect[A].Type == 112 || Effect[A].Type == 54 || Effect[A].Type == 55 || Effect[A].Type == 59 || Effect[A].Type == 77 || Effect[A].Type == 81 || Effect[A].Type == 82 || Effect[A].Type == 103 || Effect[A].Type == 104 || Effect[A].Type == 114 || Effect[A].Type == 123 || Effect[A].Type == 124)
            {
                if(vScreenCollision(Z, Effect[A].Location))
                {
                    float cn = Effect[A].Shadow ? 0.f : 1.f;
                    frmMain.renderTexture(vScreenX[Z] + Effect[A].Location.X, vScreenY[Z] + Effect[A].Location.Y, Effect[A].Location.Width, Effect[A].Location.Height, GFXEffect[Effect[A].Type], 0, Effect[A].Frame * EffectHeight[Effect[A].Type], cn, cn, cn);
                }
            }
        }


        for(A = 1; A <= numNPCs; A++) // Display NPCs that should be behind other npcs
        {
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(NPC[A].Effect == 0)
            {
                if(NPC[A].HoldingPlayer == 0 && (NPC[A].standingOnPlayer > 0 || NPC[A].Type == 56 || NPC[A].Type == 22 || NPC[A].Type == 49 || NPC[A].Type == 91 || NPC[A].Type == 160 || NPC[A].Type == 282 || NPCIsACoin[NPC[A].Type]) && (NPC[A].Generator == false || LevelEditor == true))
                {
                    // If Not NPCIsACoin(.Type) Then
                    if((vScreenCollision(Z, NPC[A].Location) | vScreenCollision(Z, newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0, NPC[A].Location.Y, static_cast<double>(NPCWidthGFX[NPC[A].Type]), static_cast<double>(NPCHeight[NPC[A].Type])))) != 0 && NPC[A].Hidden == false)
                    {
                        if(NPC[A].Active == true)
                        {
                            if(NPCWidthGFX[NPC[A].Type] == 0)
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                            }
                            else
                            {
                                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                            }
                        }
                        if(NPC[A].Reset[Z] == true || NPC[A].Active == true)
                        {
                            if(NPC[A].Active == false)
                            {
                                NPC[A].JustActivated = Z;
//                                    if(nPlay.Online == true)
//                                    {
//                                        Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                        NPC[A].JustActivated = nPlay.MySlot + 1;
//                                    }
                            }
                            NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                                if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                    timeStr = timeStr + "2b" + std::to_string(A) + LB;
                            NPC[A].Active = true;
                         }
                        NPC[A].Reset[1] = false;
                        NPC[A].Reset[2] = false;
                    }
                    else
                    {
                        NPC[A].Reset[Z] = true;
                        if(numScreens == 1)
                            NPC[A].Reset[2] = true;
                        if(SingleCoop == 1)
                            NPC[A].Reset[2] = true;
                        else if(SingleCoop == 2)
                            NPC[A].Reset[1] = true;
                    }
                    // End If
                }
            }
        }


        for(A = 1; A <= numNPCs; A++) // ice
        {
            if(NPC[A].Type == 263 && NPC[A].Effect == 0 && NPC[A].HoldingPlayer == 0)
            {
                if((vScreenCollision(Z, NPC[A].Location) | vScreenCollision(Z, newLoc(NPC[A].Location.X - (NPCWidthGFX[NPC[A].Type] - NPC[A].Location.Width) / 2.0, NPC[A].Location.Y, static_cast<double>(NPCWidthGFX[NPC[A].Type]), static_cast<double>(NPCHeight[NPC[A].Type])))) != 0 && NPC[A].Hidden == false)
                {
                    DrawFrozenNPC(Z, A);
                    if(NPC[A].Reset[Z] == true || NPC[A].Active == true)
                    {
                        if(NPC[A].Active == false)
                        {
                            NPC[A].JustActivated = Z;
//                            if(nPlay.Online == true)
//                            {
//                                Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                NPC[A].JustActivated = nPlay.MySlot + 1;
//                            }
                        }
                        NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                        if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                            timeStr = timeStr + "2b" + std::to_string(A) + LB;
                        NPC[A].Active = true;
                     }
                    NPC[A].Reset[1] = false;
                    NPC[A].Reset[2] = false;
                }
                else
                {
                    NPC[A].Reset[Z] = true;
                    if(numScreens == 1)
                        NPC[A].Reset[2] = true;
                    if(SingleCoop == 1)
                        NPC[A].Reset[2] = true;
                    else if(SingleCoop == 2)
                        NPC[A].Reset[1] = true;
                }
            }
        }


//        For A = 1 To numNPCs 'Display NPCs that should be in front of blocks
        For(A, 1, numNPCs) // Display NPCs that should be in front of blocks
        {
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(NPC[A].Effect == 0)
            {
                if(!(NPC[A].HoldingPlayer > 0 || NPCIsAVine[NPC[A].Type] == true || NPC[A].Type == 209 || NPC[A].Type == 282 || NPC[A].Type == 270 || NPC[A].Type == 160 || NPC[A].Type == 159 || NPC[A].Type == 8 || NPC[A].Type == 245 || NPC[A].Type == 93 || NPC[A].Type == 51 || NPC[A].Type == 52 || NPC[A].Type == 74 || NPC[A].Type == 256 || NPC[A].Type == 257 || NPC[A].Type == 56 || NPC[A].Type == 22 || NPC[A].Type == 49 || NPC[A].Type == 91) && !(NPC[A].Type == 45 && NPC[A].Special == 0) && NPC[A].standingOnPlayer == 0 && NPCForeground[NPC[A].Type] == false && (NPC[A].Generator == false || LevelEditor == true) && NPC[A].Type != 179 && NPC[A].Type != 263)
                {
                    if(!NPCIsACoin[NPC[A].Type])
                    {
                        if(vScreenCollision(Z, NPC[A].Location) && NPC[A].Hidden == false)
                        {
                            if(NPC[A].Type == 0)
                            {
                                NPC[A].Killed = 9;
                                KillNPC(A, 9);
                            }
                            else if(NPC[A].Active == true)
                            {
                                if(NPCIsYoshi[NPC[A].Type] == false)
                                {
                                    if(NPCWidthGFX[NPC[A].Type] == 0)
                                    {
                                        frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                                    }
                                    else
                                    {
                                        if(NPC[A].Type == 283 && NPC[A].Special > 0)
                                        {
                                            if(NPCWidthGFX[NPC[A].Special] == 0)
                                            {
                                                tempLocation.Width = NPCWidth[NPC[A].Special];
                                                tempLocation.Height = NPCHeight[NPC[A].Special];
                                            }
                                            else
                                            {
                                                tempLocation.Width = NPCWidthGFX[NPC[A].Special];
                                                tempLocation.Height = NPCHeightGFX[NPC[A].Special];
                                            }
                                            tempLocation.X = NPC[A].Location.X + NPC[A].Location.Width / 2.0 - tempLocation.Width / 2.0;
                                            tempLocation.Y = NPC[A].Location.Y + NPC[A].Location.Height / 2.0 - tempLocation.Height / 2.0;
                                            B = EditorNPCFrame(static_cast<int>(floor(static_cast<double>(NPC[A].Special))), NPC[A].Direction);
                                            frmMain.renderTexture(vScreenX[Z] + tempLocation.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC[NPC[A].Special], 0, B * tempLocation.Height);
                                        }

                                        frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                                    }
                                }
                                else
                                {
                                    if(NPC[A].Type == 95)
                                        B = 1;
                                    else if(NPC[A].Type == 98)
                                        B = 2;
                                    else if(NPC[A].Type == 99)
                                        B = 3;
                                    else if(NPC[A].Type == 100)
                                        B = 4;
                                    else if(NPC[A].Type == 148)
                                        B = 5;
                                    else if(NPC[A].Type == 149)
                                        B = 6;
                                    else if(NPC[A].Type == 150)
                                        B = 7;
                                    else if(NPC[A].Type == 228)
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
                                        if(FreezeNPCs == false)
                                            NPC[A].FrameCount = NPC[A].FrameCount + 1;
                                        if(NPC[A].FrameCount >= 70)
                                        {
                                            if(FreezeNPCs == false)
                                                NPC[A].FrameCount = 0;
                                        }
                                        else if(NPC[A].FrameCount >= 50)
                                            YoshiTFrame = 3;
                                    }
                                    else
                                    {
                                        if(FreezeNPCs == false)
                                            NPC[A].FrameCount = NPC[A].FrameCount + 1;
                                        if(NPC[A].FrameCount > 8)
                                        {
                                            YoshiBFrame = 0;
                                            NPC[A].FrameCount = 0;
                                        }
                                        else if(NPC[A].FrameCount > 6)
                                        {
                                            YoshiBFrame = 1;
                                            YoshiTX = YoshiTX - 1;
                                            YoshiTY = YoshiTY + 2;
                                            YoshiBY = YoshiBY + 1;
                                        }
                                        else if(NPC[A].FrameCount > 4)
                                        {
                                            YoshiBFrame = 2;
                                            YoshiTX = YoshiTX - 2;
                                            YoshiTY = YoshiTY + 4;
                                            YoshiBY = YoshiBY + 2;
                                        }
                                        else if(NPC[A].FrameCount > 2)
                                        {
                                            YoshiBFrame = 1;
                                            YoshiTX = YoshiTX - 1;
                                            YoshiTY = YoshiTY + 2;
                                            YoshiBY = YoshiBY + 1;
                                        }
                                        else
                                            YoshiBFrame = 0;
                                        if(FreezeNPCs == false)
                                            NPC[A].Special2 = NPC[A].Special2 + 1;
                                        if(NPC[A].Special2 > 30)
                                        {
                                            YoshiTFrame = 0;
                                            if(FreezeNPCs == false)
                                                NPC[A].Special2 = 0;
                                        }
                                        else if(NPC[A].Special2 > 10)
                                            YoshiTFrame = 2;

                                    }
                                    if(YoshiBFrame == 6)
                                    {
                                        YoshiBY = YoshiBY + 10;
                                        YoshiTY = YoshiTY + 10;
                                    }
                                    if(NPC[A].Direction == 1)
                                    {
                                        YoshiTFrame = YoshiTFrame + 5;
                                        YoshiBFrame = YoshiBFrame + 7;
                                    }
                                    else
                                    {
                                        YoshiBX = -YoshiBX;
                                        YoshiTX = -YoshiTX;
                                    }
                                    // YoshiBX = YoshiBX + 4
                                    // YoshiTX = YoshiTX + 4
                                    // Yoshi's Body
                                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(NPC[A].Location.X))) + YoshiBX, vScreenY[Z] + NPC[A].Location.Y + YoshiBY, 32, 32, GFXYoshiB[B], 0, 32 * YoshiBFrame, cn, cn, cn);

                                    // Yoshi's Head
                                    frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(NPC[A].Location.X))) + YoshiTX, vScreenY[Z] + NPC[A].Location.Y + YoshiTY, 32, 32, GFXYoshiT[B], 0, 32 * YoshiTFrame, cn, cn, cn);
                                }
                            }
                            if((NPC[A].Reset[1] && NPC[A].Reset[2]) || NPC[A].Active || NPC[A].Type == 57)
                            {
                                if(NPC[A].Active == false)
                                {
                                    NPC[A].JustActivated = Z;
//                                    if(nPlay.Online == true)
//                                    {
//                                        Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                        NPC[A].JustActivated = nPlay.MySlot + 1;
//                                    }
                                }
                                NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
                                if(NPCIsYoshi[NPC[A].Type] || NPCIsBoot[NPC[A].Type] || NPC[A].Type == 9 || NPC[A].Type == 14 || NPC[A].Type == 22 || NPC[A].Type == 90 || NPC[A].Type == 153 || NPC[A].Type == 169 || NPC[A].Type == 170 || NPC[A].Type == 182 || NPC[A].Type == 183 || NPC[A].Type == 184 || NPC[A].Type == 185 || NPC[A].Type == 186 || NPC[A].Type == 187 || NPC[A].Type == 188 || NPC[A].Type == 195 || NPC[A].Type == 104)
                                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen * 20;

//                                if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                    timeStr = timeStr + "2b" + std::to_string(A) + LB;
                                NPC[A].Active = true;
                            }
                            NPC[A].Reset[1] = false;
                            NPC[A].Reset[2] = false;
                        }
                        else
                        {
                            NPC[A].Reset[Z] = true;
                            if(numScreens == 1)
                                NPC[A].Reset[2] = true;
                            if(SingleCoop == 1)
                                NPC[A].Reset[2] = true;
                            else if(SingleCoop == 2)
                                NPC[A].Reset[1] = true;
                        }
                    }
                }
            }
//        Next A
        }

        // npc chat bubble
        for(A = 1; A <= numNPCs; A++)
        {
            if(NPC[A].Active == true && NPC[A].Chat == true)
            {
                B = NPCHeightGFX[NPC[A].Type] - NPC[A].Location.Height;
                if(B < 0)
                    B = 0;
                frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPC[A].Location.Width / 2.0 - GFX.Chat.w / 2, vScreenY[Z] + NPC[A].Location.Y - 30 - B, GFX.Chat.w, GFX.Chat.h, GFX.Chat, 0, 0);
            }
        }


        For(A, 1, numPlayers) // The clown car
        {
            if(!Player[A].Dead && !Player[A].Immune2 && Player[A].TimeToLive == 0 &&
               !(Player[A].Effect == 3 || Player[A].Effect == 5) && Player[A].Mount == 2)
            {
                 if(Player[A].Character == 1)
                 {
                     if(Player[A].State == 1)
                         Y = 24;
                     else
                         Y = 36;
                     frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64, vScreenY[Z] + Player[A].Location.Y + MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY - Y, 99, Player[A].Location.Height - 20 - Player[A].MountOffsetY, GFXMario[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                     frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 64, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128, 128, 128, GFX.Mount[Player[A].Mount], 0, 128 * Player[A].MountFrame, c, c, c);
                }
                else if(Player[A].Character == 2)
                {
                     if(Player[A].State == 1)
                         Y = 24;
                     else
                         Y = 38;
                     frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64, vScreenY[Z] + Player[A].Location.Y + LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY - Y, 99, Player[A].Location.Height - 20 - Player[A].MountOffsetY, GFXLuigi[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                     frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 64, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128, 128, 128, GFX.Mount[Player[A].Mount], 0, 128 * Player[A].MountFrame, c, c, c);
                }
                else if(Player[A].Character == 3)
                {
                     if(Player[A].State == 1)
                         Y = 24;
                     else
                         Y = 30;
                     frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64, vScreenY[Z] + Player[A].Location.Y + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY - Y, 99, Player[A].Location.Height - 20 - Player[A].MountOffsetY, GFXPeach[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                     frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 64, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128, 128, 128, GFX.Mount[Player[A].Mount], 0, 128 * Player[A].MountFrame);
                }
                else if(Player[A].Character == 4)
                {
                     if(Player[A].State == 1)
                         Y = 24;
                     else
                         Y = 30;
                     frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64, vScreenY[Z] + Player[A].Location.Y + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY - Y, 99, Player[A].Location.Height - 20 - Player[A].MountOffsetY, GFXToad[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                     frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 64, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128, 128, 128, GFX.Mount[Player[A].Mount], 0, 128 * Player[A].MountFrame, c, c, c);
                }
                else if(Player[A].Character == 5)
                {
                     Y = 30;
                     frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + LinkFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - Physics.PlayerWidth[Player[A].Character][Player[A].State] / 2 + 64, vScreenY[Z] + Player[A].Location.Y + LinkFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY - Y, 99, Player[A].Location.Height - 20 - Player[A].MountOffsetY, GFXLink[Player[A].State], pfrX(100 + Player[A].Frame * Player[A].Direction), pfrY(100 + Player[A].Frame * Player[A].Direction), c, c, c);
                     frmMain.renderTexture(vScreenX[Z] + static_cast<int>(floor(static_cast<double>(Player[A].Location.X))) + Player[A].Location.Width / 2.0 - 64, vScreenY[Z] + Player[A].Location.Y + Player[A].Location.Height - 128, 128, 128, GFX.Mount[Player[A].Mount], 0, 128 * Player[A].MountFrame, c, c, c);
                 }
            }
        }


        for(A = 1; A <= numNPCs; A++) // Put held NPCs on top
        {
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(
                (
                  (
                    (NPC[A].HoldingPlayer > 0 && Player[NPC[A].HoldingPlayer].Effect != 3) ||
                    (NPC[A].Type == 50 && NPC[A].standingOnPlayer == 0) ||
                    (NPC[A].Type == 17 && NPC[A].CantHurt > 0)
                  ) || NPC[A].Effect == 5
                ) && !(NPC[A].Type == 91) && !Player[NPC[A].HoldingPlayer].Dead
            )
            {
                if(NPC[A].Type == 263)
                {
                    DrawFrozenNPC(Z, A);
                }
                else if(NPCIsYoshi[NPC[A].Type] == false && NPC[A].Type > 0)
                {
                    if(NPCWidthGFX[NPC[A].Type] == 0)
                    {
                        frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                    }
                }
            }
        }



//'normal player draw code
//        For A = numPlayers To 1 Step -1 'Players in front of blocks
        for(int A = numPlayers; A >= 1; A--)// Players in front of blocks
        {
//            DrawPlayer A, Z
            DrawPlayer(A, Z);
//        Next A
        }
//'normal player end




//        If LevelEditor = True Or MagicHand = True Then
//            For A = 1 To numBackground 'Foreground objects
//                With Background(A)
//                    If Foreground(.Type) = True Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            If BackgroundHasNoMask(.Type) Then
//                                'BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, GFXBackgroundWidth(.Type), GFXBackgroundHeight(.Type), GFXBackground(.Type), 0, 0, vbSrcCopy
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, GFXBackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcCopy
//                            Else
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcAnd
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcPaint
//                            End If
//                        End If
//                    End If
//                End With
//            Next A
//        Else

        { // NOT AN EDITOR
            for(A = LastBackground + 1; A <= numBackground; A++) // Foreground objects
            {
                if(vScreenCollision(Z, Background[A].Location) && Background[A].Hidden == false)
                {
                    frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X, vScreenY[Z] + Background[A].Location.Y, GFXBackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackground[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                }
            }
//        End If
        }

        for(A = 1; A <= numNPCs; A++) // foreground NPCs
        {
            float cn = NPC[A].Shadow ? 0.f : 1.f;
            if(NPC[A].Effect == 0)
            {
                if(NPCForeground[NPC[A].Type] == true && NPC[A].HoldingPlayer == 0 && (NPC[A].Generator == false || LevelEditor == true))
                {
                    if(!NPCIsACoin[NPC[A].Type])
                    {
                        if(vScreenCollision(Z, NPC[A].Location) && NPC[A].Hidden == false)
                        {
                            if(NPC[A].Active == true)
                            {
                                if(NPCWidthGFX[NPC[A].Type] == 0)
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height, cn, cn, cn);
                                }
                                else
                                {
                                    frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + (NPCFrameOffsetX[NPC[A].Type] * -NPC[A].Direction) - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type], cn, cn, cn);
                                }
                            }
                            if((NPC[A].Reset[1] == true && NPC[A].Reset[2] == true) || NPC[A].Active == true)
                            {
                                if(NPC[A].Active == false)
                                {
                                    NPC[A].JustActivated = Z;
//                                    if(nPlay.Online == true)
//                                    {
//                                        Netplay::sendData "2a" + std::to_string(A) + "|" + (nPlay.MySlot + 1) + LB;
//                                        NPC[A].JustActivated = nPlay.MySlot + 1;
//                                    }
                                }
                                NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                                if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                    timeStr = timeStr + "2b" + std::to_string(A) + LB;
                                NPC[A].Active = true;
                            }
                            NPC[A].Reset[1] = false;
                            NPC[A].Reset[2] = false;
                        }
                        else
                        {
                            NPC[A].Reset[Z] = true;
                            if(numScreens == 1)
                                NPC[A].Reset[2] = true;
                            if(SingleCoop == 1)
                                NPC[A].Reset[2] = true;
                            else if(SingleCoop == 2)
                                NPC[A].Reset[1] = true;
                        }
                    }
                }
            }
        }

        for(A = fBlock; A <= lBlock; A++) // Blocks in Front
        {
            if(BlockKills[Block[A].Type])
            {
                if(vScreenCollision(Z, Block[A].Location) && Block[A].Hidden == false)
                {
                    frmMain.renderTexture(vScreenX[Z] + Block[A].Location.X, vScreenY[Z] + Block[A].Location.Y + Block[A].ShakeY3, Block[A].Location.Width, Block[A].Location.Height, GFXBlock[Block[A].Type], 0, BlockFrame[Block[A].Type] * 32);
                }
            }
        }

// effects on top
        For(A, 1, numEffects)
        {
//            With Effect(A)
            auto &e = Effect[A];
//                If .Type <> 112 And .Type <> 54 And .Type <> 55 And .Type <> 59 And .Type <> 77 And .Type <> 81 And .Type <> 82 And .Type <> 103 And .Type <> 104 And .Type <> 114 And .Type <> 123 And .Type <> 124 Then
            if(e.Type != 112 And e.Type != 54 And e.Type != 55 And e.Type != 59 And
               e.Type != 77 And e.Type != 81 And e.Type != 82 And e.Type != 103 And
               e.Type != 104 And e.Type != 114 And e.Type != 123 And e.Type != 124)
            {
//                    If vScreenCollision(Z, .Location) Then
                if(vScreenCollision(Z, e.Location))
                {
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffectMask(.Type), 0, .Frame * EffectHeight(.Type), vbSrcAnd
//                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffect(.Type), 0, .Frame * EffectHeight(.Type), vbSrcPaint
                    float c = e.Shadow ? 0.f : 1.f;
                    frmMain.renderTexture(int(vScreenX[Z] + e.Location.X), int(vScreenY[Z] + e.Location.Y),
                                          int(e.Location.Width), int(e.Location.Height),
                                          GFXEffectBMP[e.Type], 0, e.Frame * EffectHeight[e.Type], c, c, c);
//                    End If
                }
//                End If
            }
//            End With
//        Next A
        }

//        'water
//        If LevelEditor = True Then
//            For B = 1 To numWater
//                With Water(B)
//                    If .Hidden = False Then
//                        For A = 1 To .Location.Height / 32
//                            BitBlt myBackBuffer, .Location.X + vScreenX(Z), .Location.Y + vScreenY(Z) + 32 * A - 32, 2, 32, GFX.Water.hdc, 0, 0, vbSrcCopy
//                            BitBlt myBackBuffer, .Location.X + vScreenX(Z) + .Location.Width - 2, .Location.Y + vScreenY(Z) + 32 * A - 32, 2, 32, GFX.Water.hdc, 0, 0, vbSrcCopy
//                        Next A
//                        For A = 1 To .Location.Width / 32
//                            BitBlt myBackBuffer, .Location.X + vScreenX(Z) + 32 * A - 32, .Location.Y + vScreenY(Z), 32, 2, GFX.Water.hdc, 0, 0, vbSrcCopy
//                            BitBlt myBackBuffer, .Location.X + vScreenX(Z) + 32 * A - 32, .Location.Y + vScreenY(Z) + .Location.Height - 2, 32, 2, GFX.Water.hdc, 0, 0, vbSrcCopy
//                        Next A
//                    End If
//                End With
//            Next B
//        End If


        if(LevelEditor == false) // Graphics for the main game.
        {
        // NPC Generators
            for(A = 1; A <= numNPCs; A++)
            {
                if(NPC[A].Generator == true)
                {
                    if(vScreenCollision(Z, NPC[A].Location) && NPC[A].Hidden == false)
                        NPC[A].GeneratorActive = true;
                }
            }
            if(vScreen[2].Visible == true)
            {
                if(int(vScreen[Z].Width) == ScreenW)
                {
                    if(vScreen[Z].Top != 0.0)
                    {
                        frmMain.renderRect(0, 0, vScreen[Z].Width, 1, 0.f, 0.f, 0.f);
                    }
                    else
                    {
                        frmMain.renderRect(0, vScreen[Z].Height - 1, vScreen[Z].Width, 1, 0.f, 0.f, 0.f);
                    }
                }
                else
                {
                    if(vScreen[Z].Left != 0.0)
                    {
                        frmMain.renderRect(0, 0, 1, vScreen[Z].Height, 0.f, 0.f, 0.f);
                    }
                    else
                    {
                        frmMain.renderRect(vScreen[Z].Width - 1, 0, 1, vScreen[Z].Height, 0.f, 0.f, 0.f);
                    }
                }
            }

        // player names
        /* Dropped */



    //    'Interface
    //            B = 0
            B = 0;
    //            C = 0
            C = 0;
    //            If GameMenu = False And GameOutro = False Then
            if(!GameMenu && !GameOutro)
            {
    //                For A = 1 To numPlayers
                For(A, 1, numPlayers)
                {
                    if(Player[A].ShowWarp > 0)
                    {
                        std::string tempString = "";
                        if(Warp[Player[A].ShowWarp].maxStars > 0 && Player[A].Mount != 2)
                        {
                            tempString = fmt::format_ne("{0}/{0}", Warp[Player[A].ShowWarp].curStars, Warp[Player[A].ShowWarp].maxStars);
                            frmMain.renderTexture(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z] + 1, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                            frmMain.renderTexture(Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9 - 20, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z], GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                            SuperPrint(tempString, 3, Player[A].Location.X + Player[A].Location.Width / 2.0 + vScreenX[Z] - tempString.length() * 9 + 18, Player[A].Location.Y + Player[A].Location.Height - 96 + vScreenY[Z]);
                        }
                    }
                }

    //                DrawInterface Z, numScreens
                DrawInterface(Z, numScreens);

                For(A, 1, numNPCs) // Display NPCs that got dropped from the container
                {
                    if(NPC[A].Effect == 2)
                    {
                        if(NPC[A].Effect2 % 3 != 0)
                        {
                            if(vScreenCollision(Z, NPC[A].Location))
                            {
                                if(NPC[A].Active == true)
                                {
                                    if(NPCWidthGFX[NPC[A].Type] == 0)
                                    {
                                        frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type], vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type], NPC[A].Location.Width, NPC[A].Location.Height, GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPC[A].Location.Height);
                                    }
                                    else
                                    {
                                        frmMain.renderTexture(vScreenX[Z] + NPC[A].Location.X + NPCFrameOffsetX[NPC[A].Type] - NPCWidthGFX[NPC[A].Type] / 2.0 + NPC[A].Location.Width / 2.0, vScreenY[Z] + NPC[A].Location.Y + NPCFrameOffsetY[NPC[A].Type] - NPCHeightGFX[NPC[A].Type] + NPC[A].Location.Height, NPCWidthGFX[NPC[A].Type], NPCHeightGFX[NPC[A].Type], GFXNPC[NPC[A].Type], 0, NPC[A].Frame * NPCHeightGFX[NPC[A].Type]);
                                    }
                                }
                                if(NPC[A].Reset[Z] == true || NPC[A].Active == true)
                                {
                                    NPC[A].TimeLeft = Physics.NPCTimeOffScreen;
//                                    if(nPlay.Online == true && nPlay.NPCWaitCount >= 10 && nPlay.Mode == 0)
//                                        timeStr = timeStr + "2b" + std::to_string(A) + LB;
                                    NPC[A].Active = true;
                                }
                                NPC[A].Reset[1] = false;
                                NPC[A].Reset[2] = false;
                            }
                            else
                                NPC[A].Reset[Z] = true;
                        }
                    }
                }


    //                If GamePaused = True Then
                if(GamePaused)
                {
                    if(MessageText.empty())
                    {
                        X = 0;
                        Y = 0;
                        if((DScreenType == 1 && Z == 2) || (DScreenType == 2 && Z == 1))
                            X = -400;
                        else if((DScreenType == 6 && Z == 2) || (DScreenType == 4 && Z == 2) || (DScreenType == 3 && Z == 1))
                            Y = -300;
                        frmMain.renderRect(210 + X, 200 + Y, 380, 200, 0.f, 0.f, 0.f);
                        if(Cheater == false && (LevelSelect == true || (StartLevel == FileName && NoMap == true)))
                        {
                            SuperPrint("CONTINUE", 3, 272 + X, 257 + Y);
                            SuperPrint("SAVE & CONTINUE", 3, 272 + X, 292 + Y);
                            SuperPrint("SAVE & QUIT", 3, 272 + X, 327 + Y);
                            frmMain.renderTexture(252 + X, 257 + (MenuCursor * 35) + Y, 16, 16, GFX.MCursor[0], 0, 0);
                        }
                        else
                        {
                            SuperPrint("CONTINUE", 3, 272 + 56 + X, 275 + Y);
                            SuperPrint("QUIT", 3, 272 + 56 + X, 310 + Y);
                            frmMain.renderTexture(252 + 56 + X, 275 + (MenuCursor * 35) + Y, 16, 16, GFX.MCursor[0], 0, 0);
                        }
                    }
                    else
                    {
                        X = 0;
                        Y = 0;
                        if((DScreenType == 1 && Z == 2) || (DScreenType == 2 && Z == 1))
                            X = -400;
                        else if((DScreenType == 6 && Z == 2) || (DScreenType == 4 && Z == 2) || (DScreenType == 3 && Z == 1))
                            Y = -300;

                        SuperText = MessageText;
                        BoxY = 150;
                        frmMain.renderTexture(400 - GFX.TextBox.w / 2 + X,
                                              BoxY + Y + Y,
                                              GFX.TextBox.w, 20, GFX.TextBox, 0, 0);
                        BoxY = BoxY + 10;
                        tempBool = false;
                        do
                        {
                            B = 0;
                            int tempVar6 = int(SuperText.size());
                            for(A = 1; A <= tempVar6; A++)
                            {
                                if(SuperText[size_t(A) - 1] == ' ' || A == int(SuperText.size()))
                                {
                                    if(A < 28)
                                        B = A;
                                    else
                                        break;
                                }
                            }

                            if(B == 0)
                                B = A;

                            tempText = SuperText.substr(0, B);
                            SuperText = SuperText.substr(B, SuperText.length());
                            frmMain.renderTexture(400 - GFX.TextBox.w / 2 + X, BoxY + Y + Y,
                                                  GFX.TextBox.w, 20, GFX.TextBox, 0, 20);
                            if(SuperText.length() == 0 && tempBool == false)
                            {
                                SuperPrint(tempText, 4, 162 + X + (27 * 9) - (tempText.length() * 9), Y + BoxY);
                            }
                            else
                            {
                                SuperPrint(tempText, 4, 162 + X, Y + BoxY);
                            }
                            BoxY = BoxY + 16;
                            tempBool = true;
                        } while(!SuperText.empty());
                        frmMain.renderTexture(400 - GFX.TextBox.w / 2 + X, BoxY + Y + Y, GFX.TextBox.w, 10, GFX.TextBox, 0, GFX.TextBox.h - 10);
                    }
                }
    //            ElseIf GameOutro = False Then
            }

            else if(!GameOutro)
            {
                if(MenuMode != 1 && MenuMode != 2 && MenuMode != 4)
                    worldCurs = 0;

                int menuFix = -44; // for Input Settings

                frmMain.renderTexture(0, 0, GFX.MenuGFX[1].w, GFX.MenuGFX[1].h, GFX.MenuGFX[1], 0, 0);
                frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 70,
                        GFX.MenuGFX[2].w, GFX.MenuGFX[2].h, GFX.MenuGFX[2], 0, 0);

                frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[3].w / 2, 576,
                        GFX.MenuGFX[3].w, GFX.MenuGFX[3].h, GFX.MenuGFX[3], 0, 0);

                if(MenuMode == 0)
                {
                    SuperPrint("1 PLAYER GAME", 3, 300, 350);
                    SuperPrint("2 PLAYER GAME", 3, 300, 380);
                    SuperPrint("BATTLE GAME", 3, 300, 410);
                    SuperPrint("OPTIONS", 3, 300, 440);
                    SuperPrint("EXIT", 3, 300, 470);
                    frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursor[0], 0, 0);
                }
                // Character select
                else if(MenuMode == 100 Or MenuMode == 200 Or MenuMode == 300 Or MenuMode == 400 Or MenuMode == 500)
                {
                    A = 0;
                    B = 0;
                    C = 0;
                    if(blockCharacter[1] == false)
                        SuperPrint("MARIO GAME", 3, 300, 350);
                    else
                    {
                        A = A - 30;
                        if(MenuCursor + 1 >= 1)
                            B = B - 30;
                        if(PlayerCharacter >= 1)
                            C = C - 30;
                    }
                    if(blockCharacter[2] == false)
                        SuperPrint("LUIGI GAME", 3, 300, 380 + A);
                    else
                    {
                        A = A - 30;
                        if(MenuCursor + 1 >= 2)
                            B = B - 30;
                        if(PlayerCharacter >= 2)
                            C = C - 30;
                    }
                    if(blockCharacter[3] == false)
                        SuperPrint("PEACH GAME", 3, 300, 410 + A);
                    else
                    {
                        A = A - 30;
                        if(MenuCursor + 1 >= 3)
                            B = B - 30;
                        if(PlayerCharacter >= 3)
                            C = C - 30;
                    }
                    if(blockCharacter[4] == false)
                        SuperPrint("TOAD GAME", 3, 300, 440 + A);
                    else
                    {
                        A = A - 30;
                        if(MenuCursor + 1 >= 4)
                            B = B - 30;
                        if(PlayerCharacter >= 4)
                            C = C - 30;
                    }
                    if(blockCharacter[5] == false)
                        SuperPrint("LINK GAME", 3, 300, 470 + A);
                    else
                    {
                        A = A - 30;
                        if(MenuCursor + 1 >= 5)
                            B = B - 30;
                        if(PlayerCharacter >= 5)
                            C = C - 30;
                    }
                    if(MenuMode == 300 || MenuMode == 500)
                    {
                        frmMain.renderTexture(300 - 20, B + 350 + (MenuCursor * 30), GFX.MCursor[3]);
                        frmMain.renderTexture(300 - 20, B + 350 + ((PlayerCharacter - 1) * 30), GFX.MCursor[0]);
                    }
                    else
                    {
                        frmMain.renderTexture(300 - 20, B + 350 + (MenuCursor * 30), GFX.MCursor[0]);
                    }

                } else if(MenuMode == 1 Or MenuMode == 2 Or MenuMode == 4) {
                    std::string tempStr = "";
                    minShow = 1;
                    maxShow = NumSelectWorld;
                    if(NumSelectWorld > 5)
                    {
                        minShow = worldCurs;
                        maxShow = minShow + 4;

                        if(MenuCursor <= minShow - 1)
                            worldCurs = worldCurs - 1;
                        if(MenuCursor >= maxShow - 1)
                            worldCurs = worldCurs + 1;

                        if(worldCurs < 1)
                            worldCurs = 1;
                        if(worldCurs > NumSelectWorld - 4)
                            worldCurs = NumSelectWorld - 4;

                        if(maxShow >= NumSelectWorld)
                        {
                            maxShow = NumSelectWorld;
                            minShow = NumSelectWorld - 4;
                        }

                        minShow = worldCurs;
                        maxShow = minShow + 4;
                    }

                    for(auto A = minShow; A <= maxShow; A++)
                    {
                        B = A - minShow + 1;
                        tempStr = SelectWorld[A].WorldName;
                        SuperPrint(tempStr, 3, 300, 320 + (B * 30));
                    }

                    if(minShow > 1)
                    {
                        frmMain.renderTexture(400 - 8, 350 - 20, GFX.MCursor[1]);
                    }
                    if(maxShow < NumSelectWorld)
                    {
                        frmMain.renderTexture(400 - 8, 490, GFX.MCursor[2]);
                    }

                    B = MenuCursor - minShow + 1;
                    if(B >= 0 And B < 5)
                    {
                        frmMain.renderTexture(300 - 20, 350 + (B * 30), GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
                    }
                }

                else if(MenuMode == 10 Or MenuMode == 20) // Save Select
                {
                    for(auto A = 1; A <= maxSaveSlots; A++)
                    {
                        if(SaveSlot[A] >= 0)
                        {
                            SuperPrint(fmt::format_ne("SLOT {0} ... {1}", A, SaveSlot[A]), 3, 300, 320 + (A * 30));
                            if(SaveStars[A] > 0)
                            {
                                frmMain.renderTexture(560, 320 + (A * 30) + 1, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                                frmMain.renderTexture(560 + 24, 320 + (A * 30) + 2, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                                SuperPrint(fmt::format_ne("{0}", SaveStars[A]), 3, 588, 320 + (A * 30));
                            }
                        }
                        else
                        {
                            SuperPrint(fmt::format_ne("SLOT {0} ... EMPTY", A), 3, 300, 320 + (A * 30));
                        }
                    }
                    frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), GFX.MCursor[0]);
                }

                // Options Menu
                else if(MenuMode == 3)
                {
    //                    SuperPrint "PLAYER 1 CONTROLS", 3, 300, 350
                    SuperPrint("PLAYER 1 CONTROLS", 3, 300, 350);
    //                    SuperPrint "PLAYER 2 CONTROLS", 3, 300, 380
                    SuperPrint("PLAYER 2 CONTROLS", 3, 300, 380);
    //                    If resChanged = True Then
                    if(resChanged)
    //                        SuperPrint "WINDOWED MODE", 3, 300, 410
                        SuperPrint("WINDOWED MODE", 3, 300, 410);
    //                    Else
                    else
    //                        SuperPrint "FULLSCREEN MODE", 3, 300, 410
                        SuperPrint("FULLSCREEN MODE", 3, 300, 410);
    //                    End If
    //                    SuperPrint "VIEW CREDITS", 3, 300, 440
                    SuperPrint("VIEW CREDITS", 3, 300, 440);
    //                    BitBlt myBackBuffer, 300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
    //                    BitBlt myBackBuffer, 300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
                    frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30),
                                          GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
    //                ElseIf MenuMode = 31 Or MenuMode = 32 Then
                } else if(MenuMode == 31 Or MenuMode == 32) {
    //                    If useJoystick(MenuMode - 30) = 0 Then
                    if(useJoystick[MenuMode - 30] == 0)
                    {
    //                        SuperPrint "INPUT......KEYBOARD", 3, 300, 260 + menuFix
                        SuperPrint("INPUT......KEYBOARD", 3, 300, 260 + menuFix);
                        SuperPrint(fmt::format_ne("UP.........{0}", getKeyName(conKeyboard[MenuMode - 30].Up)), 3, 300, 290 + menuFix);
                        SuperPrint(fmt::format_ne("DOWN.......{0}", getKeyName(conKeyboard[MenuMode - 30].Down)), 3, 300, 320 + menuFix);
                        SuperPrint(fmt::format_ne("LEFT.......{0}", getKeyName(conKeyboard[MenuMode - 30].Left)), 3, 300, 350 + menuFix);
                        SuperPrint(fmt::format_ne("RIGHT......{0}", getKeyName(conKeyboard[MenuMode - 30].Right)), 3, 300, 380 + menuFix);
                        SuperPrint(fmt::format_ne("RUN........{0}", getKeyName(conKeyboard[MenuMode - 30].Run)), 3, 300, 410 + menuFix);
                        SuperPrint(fmt::format_ne("ALT RUN....{0}", getKeyName(conKeyboard[MenuMode - 30].AltRun)), 3, 300, 440 + menuFix);
                        SuperPrint(fmt::format_ne("JUMP.......{0}", getKeyName(conKeyboard[MenuMode - 30].Jump)), 3, 300, 470 + menuFix);
                        SuperPrint(fmt::format_ne("ALT JUMP...{0}", getKeyName(conKeyboard[MenuMode - 30].AltJump)), 3, 300, 500 + menuFix);
                        SuperPrint(fmt::format_ne("DROP ITEM..{0}", getKeyName(conKeyboard[MenuMode - 30].Drop)), 3, 300, 530 + menuFix);
                        SuperPrint(fmt::format_ne("PAUSE......{0}", getKeyName(conKeyboard[MenuMode - 30].Start)), 3, 300, 560 + menuFix);
    //                    Else
                    }
                    else
                    {
                        SuperPrint("INPUT......JOYSTICK " + std::to_string(useJoystick[MenuMode - 30]), 3, 300, 260 + menuFix);
                        if(conJoystick[MenuMode - 30].Run >= 0)
                            SuperPrint(fmt::format_ne("RUN........{0}", conJoystick[MenuMode - 30].Run), 3, 300, 290 + menuFix);
                        else
                            SuperPrint("RUN........_", 3, 300, 290 + menuFix);
                        if(conJoystick[MenuMode - 30].AltRun >= 0)
                            SuperPrint(fmt::format_ne("ALT RUN....{0}", conJoystick[MenuMode - 30].AltRun), 3, 300, 320 + menuFix);
                        else
                            SuperPrint("ALT RUN...._", 3, 300, 320 + menuFix);
                        if(conJoystick[MenuMode - 30].Jump >= 0)
                            SuperPrint(fmt::format_ne("JUMP.......{0}", conJoystick[MenuMode - 30].Jump), 3, 300, 350 + menuFix);
                        else
                            SuperPrint("JUMP......._", 3, 300, 350 + menuFix);
                        if(conJoystick[MenuMode - 30].AltJump >= 0)
                            SuperPrint(fmt::format_ne("ALT JUMP...{0}", conJoystick[MenuMode - 30].AltJump), 3, 300, 380 + menuFix);
                        else
                            SuperPrint("ALT JUMP..._", 3, 300, 380 + menuFix);
                        if(conJoystick[MenuMode - 30].Drop >= 0)
                            SuperPrint(fmt::format_ne("DROP ITEM..{0}", conJoystick[MenuMode - 30].Drop), 3, 300, 410 + menuFix);
                        else
                            SuperPrint("DROP ITEM.._", 3, 300, 410 + menuFix);
                        if(conJoystick[MenuMode - 30].Start >= 0)
                            SuperPrint(fmt::format_ne("PAUSE......{0}", conJoystick[MenuMode - 30].Start), 3, 300, 440 + menuFix);
                        else
                            SuperPrint("PAUSE......_", 3, 300, 440 + menuFix);
                    }

                    frmMain.renderTexture(300 - 20, 260 + (MenuCursor * 30) + menuFix,
                                          GFX.MCursor[0].w, GFX.MCursor[0].h, GFX.MCursor[0], 0, 0);
                }

                frmMain.renderTexture(int(MenuMouseX), int(MenuMouseY),
                                      GFX.ECursor[2].w, GFX.ECursor[2].h, GFX.ECursor[2], 0, 0);
            }
            if(PrintFPS > 0) {
                SuperPrint(fmt::format_ne("{0}", int(PrintFPS)), 1, 8, 8, 0.f, 1.f, 0.f);
            }
        }

//        If LevelEditor = True Or MagicHand = True Then
//            If LevelEditor = True Then


//            BlockFlash = BlockFlash + 1
//            If BlockFlash > 45 Then BlockFlash = 0
//            If BlockFlash <= 30 Then
//                For A = 1 To numBlock
//                    If Block(A).Special > 1000 Then
//                        If Block(A).Hidden = False Then
//                            If vScreenCollision(Z, Block(A).Location) Then
//                                If NPCWidthGFX(Block(B).Special - 1000) = 0 Then
//                                    tempLocation.X = Block(A).Location.X + Block(A).Location.Width / 2 - NPCWidth(Block(A).Special - 1000) / 2
//                                    tempLocation.Y = Block(A).Location.Y + Block(A).Location.Height / 2 - NPCHeight(Block(A).Special - 1000) / 2
//                                    tempLocation.Height = NPCHeight(Block(A).Special - 1000)
//                                    tempLocation.Width = NPCWidth(Block(A).Special - 1000)
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(Block(A).Special - 1000), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(Block(A).Special - 1000), 0, EditorNPCFrame(Block(A).Special - 1000, -1) * tempLocation.Height, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(Block(A).Special - 1000), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(Block(A).Special - 1000), 0, EditorNPCFrame(Block(A).Special - 1000, -1) * tempLocation.Height, vbSrcPaint
//                                Else
//                                    tempLocation.X = Block(A).Location.X + Block(A).Location.Width / 2 - NPCWidthGFX(Block(A).Special - 1000) / 2
//                                    tempLocation.Y = Block(A).Location.Y + Block(A).Location.Height / 2 - NPCHeightGFX(Block(A).Special - 1000) / 2
//                                    tempLocation.Height = NPCHeightGFX(Block(A).Special - 1000)
//                                    tempLocation.Width = NPCWidthGFX(Block(A).Special - 1000)
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + (NPCFrameOffsetX(Block(A).Special - 1000)) - NPCWidthGFX(Block(A).Special - 1000) / 2 + tempLocation.Width / 2, vScreenY(Z) + tempLocation.Y + NPCFrameOffsetY(Block(A).Special - 1000) - NPCHeightGFX(Block(A).Special - 1000) + tempLocation.Height, NPCWidthGFX(Block(A).Special - 1000), NPCHeightGFX(Block(A).Special - 1000), GFXNPCMask(Block(A).Special - 1000), 0, EditorNPCFrame(Block(A).Special - 1000, -1) * NPCHeightGFX(Block(A).Special - 1000), vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + (NPCFrameOffsetX(Block(A).Special - 1000)) - NPCWidthGFX(Block(A).Special - 1000) / 2 + tempLocation.Width / 2, vScreenY(Z) + tempLocation.Y + NPCFrameOffsetY(Block(A).Special - 1000) - NPCHeightGFX(Block(A).Special - 1000) + tempLocation.Height, NPCWidthGFX(Block(A).Special - 1000), NPCHeightGFX(Block(A).Special - 1000), GFXNPC(Block(A).Special - 1000), 0, EditorNPCFrame(Block(A).Special - 1000, -1) * NPCHeightGFX(Block(A).Special - 1000), vbSrcPaint
//                                End If
//                            End If
//                        End If
//                    ElseIf Block(A).Special > 0 And Block(A).Hidden = False Then
//                        tempLocation.X = Block(A).Location.X + Block(A).Location.Width / 2 - NPCWidth(10) / 2
//                        tempLocation.Y = Block(A).Location.Y + Block(A).Location.Height / 2 - NPCHeight(10) / 2
//                        tempLocation.Height = NPCHeight(10)
//                        tempLocation.Width = NPCWidth(10)
//                        BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(10), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(10), 0, EditorNPCFrame(10, -1) * tempLocation.Height, vbSrcAnd
//                        BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(10), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(10), 0, EditorNPCFrame(10, -1) * tempLocation.Height, vbSrcPaint
//                    End If
//                Next A
//                For A = 1 To numNPCs
//                    If NPC(A).Hidden = False And (NPC(A).Type = 91 Or NPC(A).Type = 96) Then
//                        If NPC(A).Special > 0 Then
//                            If vScreenCollision(Z, NPC(A).Location) Then
//                                If NPCWidthGFX(NPC(A).Special) = 0 Then
//                                    tempLocation.Height = NPCHeight(NPC(A).Special)
//                                    tempLocation.Width = NPCWidth(NPC(A).Special)
//                                    If NPC(A).Type = 96 Then
//                                        tempLocation.Y = NPC(A).Location.Y + NPC(A).Location.Height - tempLocation.Height
//                                    Else
//                                        tempLocation.Y = NPC(A).Location.Y
//                                    End If
//                                    tempLocation.X = NPC(A).Location.X + NPC(A).Location.Width / 2 - tempLocation.Width / 2
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(NPC(A).Special), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(NPC(A).Special), 0, EditorNPCFrame(Int(NPC(A).Special), -1) * tempLocation.Height, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(NPC(A).Special), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(NPC(A).Special), 0, EditorNPCFrame(Int(NPC(A).Special), -1) * tempLocation.Height, vbSrcPaint
//                                Else
//                                    tempLocation.Height = NPCHeightGFX(NPC(A).Special)
//                                    tempLocation.Width = NPCWidthGFX(NPC(A).Special)
//                                    If NPC(A).Type = 96 Then
//                                        tempLocation.Y = NPC(A).Location.Y + NPC(A).Location.Height - tempLocation.Height
//                                    Else
//                                        tempLocation.Y = NPC(A).Location.Y
//                                    End If
//                                    tempLocation.X = NPC(A).Location.X + NPC(A).Location.Width / 2 - tempLocation.Width / 2
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + (NPCFrameOffsetX(NPC(A).Special)) - NPCWidthGFX(NPC(A).Special) / 2 + tempLocation.Width / 2, vScreenY(Z) + tempLocation.Y + NPCFrameOffsetY(NPC(A).Special) - NPCHeightGFX(NPC(A).Special) + tempLocation.Height, NPCWidthGFX(NPC(A).Special), NPCHeightGFX(NPC(A).Special), GFXNPCMask(NPC(A).Special), 0, EditorNPCFrame(Int(NPC(A).Special), -1) * NPCHeightGFX(NPC(A).Special), vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + (NPCFrameOffsetX(NPC(A).Special)) - NPCWidthGFX(NPC(A).Special) / 2 + tempLocation.Width / 2, vScreenY(Z) + tempLocation.Y + NPCFrameOffsetY(NPC(A).Special) - NPCHeightGFX(NPC(A).Special) + tempLocation.Height, NPCWidthGFX(NPC(A).Special), NPCHeightGFX(NPC(A).Special), GFXNPC(NPC(A).Special), 0, EditorNPCFrame(Int(NPC(A).Special), -1) * NPCHeightGFX(NPC(A).Special), vbSrcPaint
//                                End If
//                            End If
//                        End If
//                    End If
//                Next A
//            End If








//            For A = 1 To 2 'Player start locations
//                If PlayerStart(A).Width > 0 Then
//                    C = Physics.PlayerHeight(testPlayer(A).Character, 2) - Physics.PlayerHeight(A, 2)
//                    If vScreenCollision(Z, PlayerStart(A)) Then
//                        If testPlayer(A).Character = 1 Then
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + MarioFrameX(201), vScreenY(Z) + PlayerStart(A).Y + MarioFrameY(201) - C, 99, 99, GFXMarioMask(2), 500, 0, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + MarioFrameX(201), vScreenY(Z) + PlayerStart(A).Y + MarioFrameY(201) - C, 99, 99, GFXMario(2), 500, 0, vbSrcPaint
//                        ElseIf testPlayer(A).Character = 2 Then
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + LuigiFrameX(201), vScreenY(Z) + PlayerStart(A).Y + LuigiFrameY(201) - C, 99, 99, GFXLuigiMask(2), 500, 0, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + LuigiFrameX(201), vScreenY(Z) + PlayerStart(A).Y + LuigiFrameY(201) - C, 99, 99, GFXLuigi(2), 500, 0, vbSrcPaint
//                        ElseIf testPlayer(A).Character = 3 Then
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + PeachFrameX(201), vScreenY(Z) + PlayerStart(A).Y + PeachFrameY(201) - C, 99, 99, GFXPeachMask(2), 500, 0, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + PeachFrameX(201), vScreenY(Z) + PlayerStart(A).Y + PeachFrameY(201) - C, 99, 99, GFXPeach(2), 500, 0, vbSrcPaint
//                        ElseIf testPlayer(A).Character = 4 Then
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + ToadFrameX(201), vScreenY(Z) + PlayerStart(A).Y + ToadFrameY(201) - C, 99, 99, GFXToadMask(2), 500, 0, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + ToadFrameX(201), vScreenY(Z) + PlayerStart(A).Y + ToadFrameY(201) - C, 99, 99, GFXToad(2), 500, 0, vbSrcPaint
//                        ElseIf testPlayer(A).Character = 5 Then
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + LinkFrameX(201), vScreenY(Z) + PlayerStart(A).Y + LinkFrameY(201) - C, 99, 99, GFXLinkMask(2), 500, 0, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(PlayerStart(A).X) + LinkFrameX(201), vScreenY(Z) + PlayerStart(A).Y + LinkFrameY(201) - C, 99, 99, GFXLink(2), 500, 0, vbSrcPaint
//                        End If
//                    End If
//                End If
//            Next A
//            For A = 0 To maxSections 'Show sections
//                If A <> curSection Then
//                    With tempLocation
//                        .X = level(A).X
//                        .Y = level(A).Y
//                        .Width = level(A).Width - .X
//                        .Height = level(A).Height - .Y
//                        If .X < -vScreenX(Z) Then
//                            .Width = .Width - (-vScreenX(Z) - .X)
//                            .X = -vScreenX(Z)

//                        End If
//                        If .Y < -vScreenY(Z) Then
//                            .Height = .Height - (-vScreenY(Z) - .Y)
//                            .Y = -vScreenY(Z)
//                        End If
//                        BitBlt myBackBuffer, .X + vScreenX(Z), .Y + vScreenY(Z), .Width, .Height, 0, 0, 0, vbWhiteness
//                    End With
//                End If
//            Next A
//            For A = 1 To numWarps 'Warps
//                With Warp(A)
//                    If .Direction > 0 And .Hidden = False Then
//                        If .PlacedEnt = True Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Entrance.X, vScreenY(Z) + .Entrance.Y, .Entrance.Width, .Entrance.Height, GFX.WarpMask(1).hdc, 0, 0, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Entrance.X, vScreenY(Z) + .Entrance.Y, .Entrance.Width, .Entrance.Height, GFX.Warp(1).hdc, 0, 0, vbSrcPaint
//                            SuperPrint Str(A), 1, Int(.Entrance.X + 2 + vScreenX(Z)), Int(.Entrance.Y + 2 + vScreenY(Z))
//                        End If
//                        If .PlacedExit = True Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Exit.X, vScreenY(Z) + .Exit.Y, .Exit.Width, .Exit.Height, GFX.WarpMask(1).hdc, 0, 0, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Exit.X, vScreenY(Z) + .Exit.Y, .Exit.Width, .Exit.Height, GFX.Warp(1).hdc, 0, 0, vbSrcPaint
//                            SuperPrint Str(A), 1, Int(.Exit.X + .Exit.Width - 16 - 2 + vScreenX(Z)), Int(.Exit.Y + .Exit.Height - 14 - 2 + vScreenY(Z))
//                        End If
//                    End If
//                End With
//            Next A
//            End If

//            If LevelEditor = True Then
//                If BlockFlash > 30 Or BlockFlash = 0 Then
//                    With tempLocation 'Black out the level edges
//                        .X = level(curSection).X
//                        .Y = level(curSection).Y
//                        .Width = level(curSection).Width
//                        .Height = level(curSection).Height
//                        BitBlt myBackBuffer, 0, 0, vScreenX(Z) + level(curSection).X, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, 0, 0, vScreenX(Z) + level(curSection).Width, vScreenY(Z) + level(curSection).Y, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        If -vScreenX(Z) < level(curSection).Width Then
//                            BitBlt myBackBuffer, vScreenX(Z) + level(curSection).Width, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        End If
//                        If -vScreenY(Z) < level(curSection).Height Then
//                            BitBlt myBackBuffer, 0, vScreenY(Z) + level(curSection).Height, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, GFX.Split(2).hdc, 0, 0, vbSrcCopy
//                        End If
//                    End With
//                End If
//            End If


//If nPlay.Online = True Then
//    For A = 0 To 15
//        With nPlay.Player(A)
//            If nPlay.Player(A).Active = True And nPlay.Player(A).IsMe = False Then
//                If nPlay.Player(A).Nick = "Redigit" Then
//                    nPlay.Player(A).Cursor = Int(Rnd * 8)
//                    If Rnd * 100 > 80 Then
//                        NewEffect 80, newLoc(.ECurserX, .ECurserY)
//                        Effect(numEffects).Location.SpeedX = Rnd * 4 - 2
//                        Effect(numEffects).Location.SpeedY = Rnd * 4 - 2
//                    End If
//                End If
//                BitBlt myBackBuffer, vScreenX(Z) + .ECurserX, vScreenY(Z) + .ECurserY, GFX.nCursor(.Cursor).ScaleWidth, GFX.nCursor(.Cursor).ScaleHeight, GFX.nCursorMask(.Cursor).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, vScreenX(Z) + .ECurserX, vScreenY(Z) + .ECurserY, GFX.nCursor(.Cursor).ScaleWidth, GFX.nCursor(.Cursor).ScaleHeight, GFX.nCursor(.Cursor).hdc, 0, 0, vbSrcPaint
//                SuperPrint UCase(.Nick), 3, vScreenX(Z) + .ECurserX + 28, vScreenY(Z) + .ECurserY + 34
//            End If
//        End With
//    Next A
//End If

//            If MessageText <> "" Then
//                X = 0
//                Y = 0
//                If (DScreenType = 1 And Z = 2) Or (DScreenType = 2 And Z = 1) Then
//                    X = -400
//                ElseIf (DScreenType = 6 And Z = 2) Or (DScreenType = 4 And Z = 2) Or (DScreenType = 3 And Z = 1) Then
//                    Y = -300
//                End If

//                SuperText = MessageText
//                BoxY = 150
//                BitBlt myBackBuffer, 400 - GFX.TextBox.ScaleWidth / 2 + X, BoxY + Y + Y, GFX.TextBox.ScaleWidth, 20, GFX.TextBox.hdc, 0, 0, vbSrcCopy
//                BoxY = BoxY + 10
//                tempBool = False
//                Do
//                    B = 0
//                    For A = 1 To Len(SuperText)
//                        If Mid(SuperText, A, 1) = " " Or A = Len(SuperText) Then
//                            If A < 28 Then
//                                B = A
//                            Else
//                                Exit For
//                            End If
//                        End If
//                    Next A
//                    If B = 0 Then B = A
//                    tempText = Left(SuperText, B)
//                    SuperText = Mid(SuperText, B + 1, Len(SuperText))
//                    BitBlt myBackBuffer, 400 - GFX.TextBox.ScaleWidth / 2 + X, BoxY + Y + Y, GFX.TextBox.ScaleWidth, 20, GFX.TextBox.hdc, 0, 20, vbSrcCopy
//                    If Len(SuperText) = 0 And tempBool = False Then
//                        SuperPrint tempText, 4, 162 + X + (27 * 9) - (Len(tempText) * 9), Y + BoxY
//                    Else
//                        SuperPrint tempText, 4, 162 + X, Y + BoxY
//                    End If
//                    BoxY = BoxY + 16
//                    tempBool = True
//                Loop Until Len(SuperText) = 0
//                BitBlt myBackBuffer, 400 - GFX.TextBox.ScaleWidth / 2 + X, BoxY + Y + Y, GFX.TextBox.ScaleWidth, 10, GFX.TextBox.hdc, 0, GFX.TextBox.ScaleHeight - 10, vbSrcCopy
//            End If




//            With EditorCursor 'Display the cursor
//                If .Mode = 1 Then
//                    With .Block
//                        If BlockIsSizable(.Type) Then
//                            If vScreenCollision(Z, .Location) Then
//                                For B = 0 To (.Location.Height / 32) - 1
//                                    For C = 0 To (.Location.Width / 32) - 1
//                                        D = C
//                                        E = B
//                                        If Not D = 0 Then
//                                            If D = (.Location.Width / 32) - 1 Then
//                                                D = 2
//                                            Else
//                                                D = 1
//                                                d2 = 0.5
//                                            End If
//                                        End If
//                                        If Not E = 0 Then
//                                            If E = (.Location.Height / 32) - 1 Then
//                                                E = 2
//                                            Else
//                                                E = 1
//                                            End If
//                                        End If
//                                        If (D = 0 Or D = 2) Or (E = 0 Or E = 2) Then
//                                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + C * 32, vScreenY(Z) + .Location.Y + B * 32, 32, 32, GFXBlockMask(.Type), D * 32, E * 32, vbSrcAnd
//                                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + C * 32, vScreenY(Z) + .Location.Y + B * 32, 32, 32, GFXBlock(.Type), D * 32, E * 32, vbSrcPaint
//                                        Else
//                                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + C * 32, vScreenY(Z) + .Location.Y + B * 32, 32, 32, GFXBlock(.Type), D * 32, E * 32, vbSrcCopy
//                                        End If
//                                    Next C
//                                Next B
//                            End If
//                        End If
//                    End With
//                With .Block
//                    If Not BlockIsSizable(.Type) Then
//                        If vScreenCollision(Z, .Location) Then
//                            If BlockHasNoMask(.Type) = True Then
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .ShakeY3, .Location.Width, .Location.Height, GFXBlock(.Type), 0, BlockFrame(.Type) * 32, vbSrcCopy
//                            Else
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .ShakeY3, .Location.Width, .Location.Height, GFXBlockMask(.Type), 0, BlockFrame(.Type) * 32, vbSrcAnd
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .ShakeY3, .Location.Width, .Location.Height, GFXBlock(.Type), 0, BlockFrame(.Type) * 32, vbSrcPaint
//                            End If
//                        End If
//                    End If
//                End With
//                ElseIf .Mode = 2 Then
//                    If frmLevelSettings.optLevel(4).Value = True Or frmLevelSettings.optLevel(5).Value = True Then
//                        If frmLevelSettings.optLevel(4).Value = True Then
//                            A = 1
//                        Else
//                            A = 2
//                        End If
//                        C = Physics.PlayerHeight(testPlayer(A).Character, 2) - Physics.PlayerHeight(A, 2)
//                        With EditorCursor
//                            If vScreenCollision(Z, .Location) Then
//                                If testPlayer(A).Character = 1 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + MarioFrameX(201), vScreenY(Z) + .Location.Y + MarioFrameY(201) - C, 99, 99, GFXMarioMask(2), 500, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + MarioFrameX(201), vScreenY(Z) + .Location.Y + MarioFrameY(201) - C, 99, 99, GFXMario(2), 500, 0, vbSrcPaint
//                                ElseIf testPlayer(A).Character = 2 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX(201), vScreenY(Z) + .Location.Y + LuigiFrameY(201) - C, 99, 99, GFXLuigiMask(2), 500, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX(201), vScreenY(Z) + .Location.Y + LuigiFrameY(201) - C, 99, 99, GFXLuigi(2), 500, 0, vbSrcPaint
//                                ElseIf testPlayer(A).Character = 3 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + PeachFrameX(201), vScreenY(Z) + .Location.Y + PeachFrameY(201) - C, 99, 99, GFXPeachMask(2), 500, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + PeachFrameX(201), vScreenY(Z) + .Location.Y + PeachFrameY(201) - C, 99, 99, GFXPeach(2), 500, 0, vbSrcPaint
//                                ElseIf testPlayer(A).Character = 4 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + ToadFrameX(201), vScreenY(Z) + .Location.Y + ToadFrameY(201) - C, 99, 99, GFXToadMask(2), 500, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + ToadFrameX(201), vScreenY(Z) + .Location.Y + ToadFrameY(201) - C, 99, 99, GFXToad(2), 500, 0, vbSrcPaint
//                                ElseIf testPlayer(A).Character = 5 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LinkFrameX(201), vScreenY(Z) + .Location.Y + LinkFrameY(201) - C, 99, 99, GFXLinkMask(2), 500, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LinkFrameX(201), vScreenY(Z) + .Location.Y + LinkFrameY(201) - C, 99, 99, GFXLink(2), 500, 0, vbSrcPaint
//                                End If
//                            End If
//                        End With
//                    End If

//                ElseIf .Mode = 3 Then
//                    With .Background
//                        If vScreenCollision(Z, .Location) Then
//                            If BackgroundHasNoMask(.Type) Then
//                                BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcCopy
//                            Else
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcPaint
//                            End If
//                        End If
//                    End With
//                ElseIf .Mode = 4 Then
//                    .NPC.Frame = NPC(0).Frame
//                    .NPC.FrameCount = NPC(0).FrameCount
//                    NPC(0) = .NPC
//                    NPCFrames 0
//                    .NPC = NPC(0)

//                    With .NPC
//                        If NPCWidthGFX(.Type) = 0 Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * .Location.Height, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * .Location.Height, vbSrcPaint
//                        Else
//                            If .Type = 283 And .Special > 0 Then
//                                If NPCWidthGFX(.Special) = 0 Then
//                                    tempLocation.Width = NPCWidth(.Special)
//                                    tempLocation.Height = NPCHeight(.Special)
//                                Else
//                                    tempLocation.Width = NPCWidthGFX(.Special)
//                                    tempLocation.Height = NPCHeightGFX(.Special)
//                                End If
//                                tempLocation.X = .Location.X + .Location.Width / 2 - tempLocation.Width / 2
//                                tempLocation.Y = .Location.Y + .Location.Height / 2 - tempLocation.Height / 2
//                                B = EditorNPCFrame(Int(.Special), .Direction)
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(.Type), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(.Special), 0, B * tempLocation.Height, vbSrcAnd
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(.Type), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(.Special), 0, B * tempLocation.Height, vbSrcPaint
//                            End If

//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPCMask(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPC(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcPaint
//                        End If
//                    End With
//                End If
//                If .Mode = 0 Or .Mode = 6 Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 2, vScreenY(Z) + .Location.Y, 22, 30, GFX.ECursorMask(3).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 2, vScreenY(Z) + .Location.Y, 22, 30, GFX.ECursor(3).hdc, 0, 0, vbSrcPaint
//                ElseIf .Mode = 13 Or .Mode = 14 Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, 32, 32, GFX.ECursorMask(2).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, 32, 32, GFX.ECursor(2).hdc, 0, 0, vbSrcPaint
//                ElseIf .Mode = 2 And (frmLevelSettings.optLevel(0).Value = True Or frmLevelSettings.optLevel(1).Value = True Or frmLevelSettings.optLevel(2).Value = True Or frmLevelSettings.optLevel(3).Value = True) Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, 32, 32, GFX.ECursorMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, 32, 32, GFX.ECursor(1).hdc, 0, 0, vbSrcPaint

//                ElseIf .Mode = 15 Then 'Water
//                    For A = 1 To .Location.Height / 32
//                        BitBlt myBackBuffer, .Location.X + vScreenX(Z), .Location.Y + vScreenY(Z) + 32 * A - 32, 2, 32, GFX.Water.hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, .Location.X + vScreenX(Z) + .Location.Width - 2, .Location.Y + vScreenY(Z) + 32 * A - 32, 2, 32, GFX.Water.hdc, 0, 0, vbSrcCopy
//                    Next A
//                    For A = 1 To .Location.Width / 32
//                        BitBlt myBackBuffer, .Location.X + vScreenX(Z) + 32 * A - 32, .Location.Y + vScreenY(Z), 32, 2, GFX.Water.hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, .Location.X + vScreenX(Z) + 32 * A - 32, .Location.Y + vScreenY(Z) + .Location.Height - 2, 32, 2, GFX.Water.hdc, 0, 0, vbSrcCopy
//                    Next A
//                    BitBlt myBackBuffer, .X - vScreen(Z).Left, .Y - vScreen(Z).Top, 32, 32, GFX.ECursorMask(2).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, .X - vScreen(Z).Left, .Y - vScreen(Z).Top, 32, 32, GFX.ECursor(2).hdc, 0, 0, vbSrcPaint
//                    If LCase(frmLayers.lstLayer.List(frmLayers.lstLayer.ListIndex)) <> "default" Then
//                        SuperPrint UCase(frmLayers.lstLayer.List(frmLayers.lstLayer.ListIndex)), 3, .X + 28, .Y + 34
//                    End If
//                Else
//                    If .Mode = 5 Then
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, 2, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .Location.Height - 2, .Location.Width, 2, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, 2, .Location.Height, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + .Location.Width - 2, vScreenY(Z) + .Location.Y, 2, .Location.Height, GFX.Split(1).hdc, 0, 0, vbSrcCopy
//                    End If
//                    BitBlt myBackBuffer, .X - vScreen(Z).Left, .Y - vScreen(Z).Top, 32, 32, GFX.ECursorMask(2).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, .X - vScreen(Z).Left, .Y - vScreen(Z).Top, 32, 32, GFX.ECursor(2).hdc, 0, 0, vbSrcPaint
//                    If LCase(frmLayers.lstLayer.List(frmLayers.lstLayer.ListIndex)) <> "default" Then
//                        SuperPrint UCase(frmLayers.lstLayer.List(frmLayers.lstLayer.ListIndex)), 3, .X + 28, .Y + 34
//                    End If
//                End If
//            End With
//        End If


        if(numScreens > 1) // for multiple screens
            frmMain.resetViewport();

        if(GameOutro)
        {
            frmMain.renderRect(0, 0, 800, CreditChop, 0.f, 0.f, 0.f);
            frmMain.renderRect(0, 600 - CreditChop, 800, 600, 0.f, 0.f, 0.f);
            DoCredits();
        }

//        If LevelEditor = True Then
//            StretchBlt frmLevelWindow.vScreen(Z).hdc, 0, 0, frmLevelWindow.vScreen(Z).ScaleWidth, frmLevelWindow.vScreen(Z).ScaleHeight, myBackBuffer, 0, 0, 800, 600, vbSrcCopy
//        Else
        { // NOT AN EDITOR!!!
            if(ScreenShake > 0)
            {
                ScreenShake--;
                A = (std::rand() % ScreenShake * 4) - ScreenShake * 2;
                B = (std::rand() % ScreenShake * 4) - ScreenShake * 2;
                frmMain.offsetViewport(A, B);
                if(ScreenShake == 0)
                    frmMain.offsetViewport(0, 0);
            }
            frmMain.repaint();
        }
//    Next Z
    }

//    If TakeScreen = True Then ScreenShot
    if(TakeScreen)
        ScreenShot();

    // Update Coin Frames
    CoinFrame2[1] = CoinFrame2[1] + 1;
    if(CoinFrame2[1] >= 6)
    {
        CoinFrame2[1] = 0;
        CoinFrame[1] = CoinFrame[1] + 1;
        if(CoinFrame[1] >= 4)
            CoinFrame[1] = 0;
    }
    CoinFrame2[2] = CoinFrame2[2] + 1;
    if(CoinFrame2[2] >= 6)
    {
        CoinFrame2[2] = 0;
        CoinFrame[2] = CoinFrame[2] + 1;
        if(CoinFrame[2] >= 7)
            CoinFrame[2] = 0;
    }
    CoinFrame2[3] = CoinFrame2[3] + 1;
    if(CoinFrame2[3] >= 7)
    {
        CoinFrame2[3] = 0;
        CoinFrame[3] = CoinFrame[3] + 1;
        if(CoinFrame[3] >= 4)
            CoinFrame[3] = 0;
    }
//    if(nPlay.Mode == 0)
//    {
//        if(nPlay.NPCWaitCount >= 11)
//            nPlay.NPCWaitCount = 0;
//        nPlay.NPCWaitCount = nPlay.NPCWaitCount + 2;
//        if(timeStr != "")
//            Netplay::sendData timeStr + LB;
//    }
}

//  Get the screen position
void GetvScreen(int A)
{
    // Netplay code, disabled
//    if(ScreenType == 8)
//    {
//        if(Player[nPlay.MySlot + 1].Mount == 2)
//            Player[nPlay.MySlot + 1].Location.Height = 0;
//        vScreenX[1] = -Player[nPlay.MySlot + 1].Location.X + (vScreen[1].Width * 0.5) - Player[nPlay.MySlot + 1].Location.Width / 2.0;
//        vScreenY[1] = -Player[nPlay.MySlot + 1].Location.Y + (vScreen[1].Height * 0.5) - vScreenYOffset - Player[nPlay.MySlot + 1].Location.Height;
//        vScreenX[1] = vScreenX[1] - vScreen[1].tempX;
//        vScreenY[1] = vScreenY[1] - vScreen[1].TempY;
//        if(-vScreenX[1] < level[Player[nPlay.MySlot + 1].Section].X)
//            vScreenX[1] = -level[Player[nPlay.MySlot + 1].Section].X;
//        if(-vScreenX[1] + vScreen[1].Width > level[Player[nPlay.MySlot + 1].Section].Width)
//            vScreenX[1] = -(level[Player[nPlay.MySlot + 1].Section].Width - vScreen[1].Width);
//        if(-vScreenY[1] < level[Player[nPlay.MySlot + 1].Section].Y)
//            vScreenY[1] = -level[Player[nPlay.MySlot + 1].Section].Y;
//        if(-vScreenY[1] + vScreen[1].Height > level[Player[nPlay.MySlot + 1].Section].Height)
//            vScreenY[1] = -(level[Player[nPlay.MySlot + 1].Section].Height - vScreen[1].Height);
//        if(vScreen[1].TempDelay > 0)
//            vScreen[1].TempDelay = vScreen[1].TempDelay - 1;
//        else
//        {
//            if(vScreen[1].tempX > 0)
//                vScreen[1].tempX = vScreen[1].tempX - 1;
//            if(vScreen[1].tempX < 0)
//                vScreen[1].tempX = vScreen[1].tempX + 1;
//            if(vScreen[1].TempY > 0)
//                vScreen[1].TempY = vScreen[1].TempY - 1;
//            if(vScreen[1].TempY < 0)
//                vScreen[1].TempY = vScreen[1].TempY + 1;
//        }
//        if(Player[nPlay.MySlot + 1].Mount == 2)
//            Player[nPlay.MySlot + 1].Location.Height = 128;
//    }
//    else
    {
        if(Player[A].Mount == 2)
            Player[A].Location.Height = 0;
        vScreenX[A] = -Player[A].Location.X + (vScreen[A].Width * 0.5) - Player[A].Location.Width / 2.0;
        vScreenY[A] = -Player[A].Location.Y + (vScreen[A].Height * 0.5) - vScreenYOffset - Player[A].Location.Height;
        vScreenX[A] = vScreenX[A] - vScreen[A].tempX;
        vScreenY[A] = vScreenY[A] - vScreen[A].TempY;
        if(-vScreenX[A] < level[Player[A].Section].X)
            vScreenX[A] = -level[Player[A].Section].X;
        if(-vScreenX[A] + vScreen[A].Width > level[Player[A].Section].Width)
            vScreenX[A] = -(level[Player[A].Section].Width - vScreen[A].Width);
        if(-vScreenY[A] < level[Player[A].Section].Y)
            vScreenY[A] = -level[Player[A].Section].Y;
        if(-vScreenY[A] + vScreen[A].Height > level[Player[A].Section].Height)
            vScreenY[A] = -(level[Player[A].Section].Height - vScreen[A].Height);
        if(vScreen[A].TempDelay > 0)
            vScreen[A].TempDelay = vScreen[A].TempDelay - 1;
        else
        {
            if(vScreen[A].tempX > 0)
                vScreen[A].tempX = vScreen[A].tempX - 1;
            if(vScreen[A].tempX < 0)
                vScreen[A].tempX = vScreen[A].tempX + 1;
            if(vScreen[A].TempY > 0)
                vScreen[A].TempY = vScreen[A].TempY - 1;
            if(vScreen[A].TempY < 0)
                vScreen[A].TempY = vScreen[A].TempY + 1;
        }
        if(Player[A].Mount == 2)
            Player[A].Location.Height = 128;
    }
}

// Get the average screen position for all players
void GetvScreenAverage()
{
    int A = 0;
    int B = 0;
    double OldX = 0;
    double OldY = 0;
    OldX = vScreenX[1];
    OldY = vScreenY[1];
    vScreenX[1] = 0;
    vScreenY[1] = 0;
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Dead == false && Player[A].Effect != 6)
        {
            vScreenX[1] = vScreenX[1] - Player[A].Location.X - Player[A].Location.Width / 2.0;
            if(Player[A].Mount == 2)
                vScreenY[1] = vScreenY[1] - Player[A].Location.Y;
            else
                vScreenY[1] = vScreenY[1] - Player[A].Location.Y - Player[A].Location.Height;
            B = B + 1;
        }
    }
    A = 1;
    if(B == 0)
    {
        if(GameMenu == true)
        {
            vScreenX[1] = -level[0].X;
            B = 1;
        }
        else
            return;
    }
    vScreenX[1] = (vScreenX[1] / B) + (ScreenW * 0.5);
    vScreenY[1] = (vScreenY[1] / B) + (ScreenH * 0.5) - vScreenYOffset;
    if(-vScreenX[A] < level[Player[1].Section].X)
        vScreenX[A] = -level[Player[1].Section].X;
    if(-vScreenX[A] + ScreenW > level[Player[1].Section].Width)
        vScreenX[A] = -(level[Player[1].Section].Width - ScreenW);
    if(-vScreenY[A] < level[Player[1].Section].Y)
        vScreenY[A] = -level[Player[1].Section].Y;
    if(-vScreenY[A] + ScreenH > level[Player[1].Section].Height)
        vScreenY[A] = -(level[Player[1].Section].Height - ScreenH);
    if(GameMenu == true)
    {
        if(vScreenX[1] > OldX)
        {
            if(vScreenX[1] == -level[0].X)
                vScreenX[1] = OldX + 20;
            else
                vScreenX[1] = OldX;
        }
        else if(vScreenX[1] < OldX - 10)
            vScreenX[1] = OldX - 10;
    }
}

// Get the average screen position for all players with no level edge detection
void GetvScreenAverage2()
{
    int A = 0;
    int B = 0;
    vScreenX[1] = 0;
    vScreenY[1] = 0;
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Dead == false)
        {
            vScreenX[1] = vScreenX[1] - Player[A].Location.X - Player[A].Location.Width / 2.0;
            if(Player[A].Mount == 2)
                vScreenY[1] = vScreenY[1] - Player[A].Location.Y;
            else
                vScreenY[1] = vScreenY[1] - Player[A].Location.Y - Player[A].Location.Height;
            B = B + 1;
        }
    }
    A = 1;
    if(B == 0)
        return;
    vScreenX[1] = (vScreenX[1] / B) + (ScreenW * 0.5);
    vScreenY[1] = (vScreenY[1] / B) + (ScreenH * 0.5) - vScreenYOffset;
}

void SetupGraphics()
{
    //DUMMY AND USELESS

    // Creates the back buffer for the main game
    // myBackBuffer = CreateCompatibleDC(GetDC(0))
    // myBufferBMP = CreateCompatibleBitmap(GetDC(0), screenw, screenh)
//    myBackBuffer = CreateCompatibleDC(frmMain::hdc);
//    myBufferBMP = CreateCompatibleBitmap(frmMain::hdc, ScreenW, ScreenH);
//    SelectObject myBackBuffer, myBufferBMP;
//    GFX.Split(2).Width = ScreenW;
//    GFX.Split(2).Height = ScreenH;
    // GFX.BackgroundColor(1).Width = Screen.Width
    // GFX.BackgroundColor(1).Height = Screen.Height
    // GFX.BackgroundColor(2).Width = Screen.Width
    // GFX.BackgroundColor(2).Height = Screen.Height
}

void SetupEditorGraphics()
{
    //DUMMY AND USELESS

//    GFX.Split(1).Width = frmLevelWindow.vScreen(1).Width
//    GFX.Split(1).Height = frmLevelWindow.vScreen(1).Height
//    GFX.Split(2).Width = frmLevelWindow.vScreen(1).Width
//    GFX.Split(2).Height = frmLevelWindow.vScreen(1).Height
//    vScreen(1).Height = frmLevelWindow.vScreen(1).ScaleHeight
//    vScreen(1).Width = frmLevelWindow.vScreen(1).ScaleWidth
//    vScreen(1).Left = 0
//    vScreen(1).Top = 0
//    vScreen(2).Visible = False
//    'Creates the back buffer for the level editor
//    'myBackBuffer = CreateCompatibleDC(GetDC(0))
//    'myBufferBMP = CreateCompatibleBitmap(GetDC(0), screenw, screenh)
//    'SelectObject myBackBuffer, myBufferBMP
//    GFX.BackgroundColor(1).Width = frmLevelWindow.vScreen(1).Width
//    GFX.BackgroundColor(1).Height = frmLevelWindow.vScreen(1).Height
//    GFX.BackgroundColor(2).Width = frmLevelWindow.vScreen(1).Width
//    GFX.BackgroundColor(2).Height = frmLevelWindow.vScreen(1).Height
}

// Sets up the split lines
void SetupScreens()
{
    switch(ScreenType)
    {
    case 0: // Follows Player 1
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = False;
        break;
    case 1: // Split Screen vertical
        vScreen[1].Height = ScreenH / 2;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Height = ScreenH / 2;
        vScreen[2].Width = ScreenW;
        vScreen[2].Left = 0;
        vScreen[2].Top = ScreenH / 2;
        break;
    case 2: // Follows all players
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = False;
        break;
    case 3: // Follows all players. Noone leaves the screen
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = False;
        break;
    case 4: // Split Screen horizontal
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW / 2;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Height = ScreenH;
        vScreen[2].Width = ScreenW / 2;
        vScreen[2].Left = ScreenW / 2;
        vScreen[2].Top = 0;
        break;
    case 5: // Dynamic screen detection
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = False;
        break;
    case 6: // VScreen Coop
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Height = ScreenH;
        vScreen[2].Width = ScreenW;
        vScreen[2].Left = 0;
        vScreen[2].Top = 0;
        break;
    case 7:
        vScreen[1].Left = 0;
        vScreen[1].Width = 800;
        vScreen[1].Top = 0;
        vScreen[1].Height = 600;
        vScreen[2].Visible = False;
        break;
    case 8: // netplay
        vScreen[1].Left = 0;
        vScreen[1].Width = 800;
        vScreen[1].Top = 0;
        vScreen[1].Height = 600;
        vScreen[2].Visible = False;
        break;
//    End If
    }
}

void DynamicScreen()
{
    int A = 0;
    GetvScreenAverage();

    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Effect == 6)
            return;
    }

    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 2)
            Player[A].Location.Height = 0;
    }

    if(CheckDead() == 0)
    {
        if(Player[1].Section == Player[2].Section)
        {
            if(level[Player[1].Section].Width - level[Player[1].Section].X > ScreenW && (((vScreen[2].Visible == false && Player[2].Location.X + vScreenX[1] >= ScreenW * 0.75 - Player[2].Location.Width / 2.0) || (vScreen[2].Visible == true && Player[2].Location.X + vScreenX[1] >= ScreenW * 0.75 - Player[2].Location.Width / 2.0)) && (Player[1].Location.X < level[Player[1].Section].Width - ScreenW * 0.75 - Player[1].Location.Width / 2.0)))
            {
                vScreen[2].Height = ScreenH;
                vScreen[2].Width = ScreenW / 2;
                vScreen[2].Left = ScreenW / 2.0;
                vScreen[2].Top = 0;
                vScreen[1].Height = ScreenH;
                vScreen[1].Width = ScreenW / 2;
                vScreen[1].Left = 0;
                vScreen[1].Top = 0;
                GetvScreenAverage2();
                if(DScreenType != 1)
                    PlaySound(13);
                for(A = 1; A <= 2; A++)
                {
                    vScreen[A].TempDelay = 200;
                    vScreen[A].tempX = 0;
                    vScreen[A].TempY = -vScreenY[1] + ScreenH * 0.5 - Player[A].Location.Y - vScreenYOffset - Player[A].Location.Height;
                }
                vScreen[2].Visible = true;
                DScreenType = 1;
            }
            else if(level[Player[1].Section].Width - level[Player[1].Section].X > ScreenW && (((vScreen[2].Visible == false && Player[1].Location.X + vScreenX[1] >= ScreenW * 0.75 - Player[1].Location.Width / 2.0) || (vScreen[2].Visible == true && Player[1].Location.X + vScreenX[2] >= ScreenW * 0.75 - Player[1].Location.Width / 2.0)) && (Player[2].Location.X < level[Player[1].Section].Width - ScreenW * 0.75 - Player[2].Location.Width / 2.0)))
            {
                vScreen[1].Height = ScreenH;
                vScreen[1].Width = ScreenW / 2;
                vScreen[1].Left = ScreenW / 2.0;
                vScreen[1].Top = 0;
                vScreen[2].Height = ScreenH;
                vScreen[2].Width = ScreenW / 2;
                vScreen[2].Left = 0;
                vScreen[2].Top = 0;
                GetvScreenAverage2();
                if(DScreenType != 2)
                    PlaySound(13);
                for(A = 1; A <= 2; A++)
                {
                    vScreen[A].TempDelay = 200;
                    vScreen[A].tempX = 0;
                    vScreen[A].TempY = -vScreenY[1] + ScreenH * 0.5 - Player[A].Location.Y - vScreenYOffset - Player[A].Location.Height;
                }
                DScreenType = 2;
                vScreen[2].Visible = true;
            }
            else if(level[Player[1].Section].Height - level[Player[1].Section].Y > ScreenH && ((vScreen[2].Visible == false && Player[1].Location.Y + vScreenY[1] >= ScreenH * 0.75 - vScreenYOffset - Player[1].Location.Height) || (vScreen[2].Visible == true && Player[1].Location.Y + vScreenY[2] >= ScreenH * 0.75 - vScreenYOffset - Player[1].Location.Height)) && (Player[2].Location.Y < level[Player[1].Section].Height - ScreenH * 0.75 - vScreenYOffset - Player[2].Location.Height))
            {
                vScreen[1].Height = ScreenH / 2;
                vScreen[1].Width = ScreenW;
                vScreen[1].Left = 0;
                vScreen[1].Top = ScreenH / 2.0;
                vScreen[2].Height = ScreenH / 2;
                vScreen[2].Width = ScreenW;
                vScreen[2].Left = 0;
                vScreen[2].Top = 0;
                GetvScreenAverage2();
                if(DScreenType != 3)
                    PlaySound(13);
                for(A = 1; A <= 2; A++)
                {
                    vScreen[A].TempDelay = 200;
                    vScreen[A].TempY = 0;
                    vScreen[A].tempX = -vScreenX[1] + ScreenW * 0.5 - Player[A].Location.X - Player[A].Location.Width * 0.5;
                }
                vScreen[2].Visible = true;
                DScreenType = 3;
            }
            else if(level[Player[1].Section].Height - level[Player[1].Section].Y > ScreenH && ((vScreen[2].Visible == false && Player[2].Location.Y + vScreenY[1] >= ScreenH * 0.75 - vScreenYOffset - Player[2].Location.Height) || (vScreen[2].Visible == true && Player[2].Location.Y + vScreenY[1] >= ScreenH * 0.75 - vScreenYOffset - Player[2].Location.Height)) && (Player[1].Location.Y < level[Player[1].Section].Height - ScreenH * 0.75 - vScreenYOffset - Player[1].Location.Height))
            {
                vScreen[1].Height = ScreenH / 2;
                vScreen[1].Width = ScreenW;
                vScreen[1].Left = 0;
                vScreen[1].Top = 0;
                vScreen[2].Height = ScreenH / 2;
                vScreen[2].Width = ScreenW;
                vScreen[2].Left = 0;
                vScreen[2].Top = ScreenH / 2.0;
                GetvScreenAverage2();
                if(DScreenType != 4)
                    PlaySound(13);
                for(A = 1; A <= 2; A++)
                {
                    vScreen[A].TempDelay = 200;
                    vScreen[A].TempY = 0;
                    vScreen[A].tempX = -vScreenX[1] + ScreenW * 0.5 - Player[A].Location.X - Player[A].Location.Width * 0.5;
                }
                vScreen[2].Visible = true;
                DScreenType = 4;
            }
            else
            {
                if(vScreen[2].Visible == true)
                {
                    if(DScreenType != 5)
                        PlaySound(13);
                    vScreen[2].Visible = false;
                    vScreen[1].Height = ScreenH;
                    vScreen[1].Width = ScreenW;
                    vScreen[1].Left = 0;
                    vScreen[1].Top = 0;
                    vScreen[1].tempX = 0;
                    vScreen[1].TempY = 0;
                    vScreen[2].tempX = 0;
                    vScreen[2].TempY = 0;
                }
                DScreenType = 5;
            }
            for(A = 1; A <= 2; A++)
            {
                if(vScreen[A].TempY > (vScreen[A].Height * 0.25))
                    vScreen[A].TempY = (vScreen[A].Height * 0.25);
                if(vScreen[A].TempY < -(vScreen[A].Height * 0.25))
                    vScreen[A].TempY = -(vScreen[A].Height * 0.25);
                if(vScreen[A].tempX > (vScreen[A].Width * 0.25))
                    vScreen[A].tempX = (vScreen[A].Width * 0.25);
                if(vScreen[A].tempX < -(vScreen[A].Width * 0.25))
                    vScreen[A].tempX = -(vScreen[A].Width * 0.25);
            }
        }
        else
        {
            vScreen[1].Height = ScreenH / 2;
            vScreen[1].Width = ScreenW;
            vScreen[1].Left = 0;
            vScreen[1].Top = 0;
            vScreen[2].Height = ScreenH / 2;
            vScreen[2].Width = ScreenW;
            vScreen[2].Left = 0;
            vScreen[2].Top = ScreenH / 2.0;
            vScreen[1].tempX = 0;
            vScreen[1].TempY = 0;
            vScreen[2].tempX = 0;
            vScreen[2].TempY = 0;
            GetvScreenAverage2();
            if(DScreenType != 6)
                PlaySound(13);
            DScreenType = 6;
            vScreen[2].Visible = true;
        }
    }
    else
    {
        if(vScreen[2].Visible == true)
        {
            vScreen[2].Visible = false;
            vScreen[1].Visible = false;
            vScreen[1].Height = ScreenH;
            vScreen[1].Width = ScreenW;
            vScreen[1].Left = 0;
            vScreen[1].Top = 0;
            vScreen[1].Visible = true;
            vScreen[1].tempX = 0;
            vScreen[1].TempY = 0;
            vScreen[2].tempX = 0;
            vScreen[2].TempY = 0;
        }
    }
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Mount == 2)
            Player[A].Location.Height = 128;
    }
}

void SuperPrint(std::string SuperWords, int Font, float X, float Y,
                float r, float g, float b, float a)
{
//    int A = 0;
    int B = 0;
    int C = 0;

    if(Font == 1)
    {
        for(auto c : SuperWords)
        {
            if(c >= '0' && c <= '9')
                frmMain.renderTexture(int(X + B), int(Y), 16, 14, GFX.Font1[c - '0'], 0, 0, r, g, b, a);
            B += 18;
        }
    }
    else if(Font == 2)
    {
        for(auto c : SuperWords)
        {
            if(c >= 48 && c <= 57) {
                C = (c - 48) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 65 && c <= 90) {
                C = (c - 55) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 97 && c <= 122) {
                C = (c - 61) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 33 && c <= 47) {
                C = (c - 33) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 58 && c <= 64) {
                C = (c - 58 + 15) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 91 && c <= 96) {
                C = (c - 91 + 22) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else if(c >= 123 && c <= 125) {
                C = (c - 123 + 28) * 16;
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0, r, g, b, a);
                B += 16;
            } else {
                B += 16;
            }
        }

    }
    else if (Font == 3)
    {
        std::string Words = SuperWords;
        std::transform(Words.begin(), Words.end(), Words.begin(), [](unsigned char c){ return std::toupper(c); });
//        Do While Len(Words) > 0
        for(auto c : Words)
        {
//            If Asc(Left(Words, 1)) >= 33 And Asc(Left(Words, 1)) <= 126 Then
            if(c >= 33 && c <= 126)
            {
//                C = (Asc(Left(Words, 1)) - 33) * 32
                C = (c - 33) * 32;
//                BitBlt myBackBuffer, X + B, Y, 18, 16, GFX.Font2Mask(2).hdc, 2, C, vbSrcAnd
//                BitBlt myBackBuffer, X + B, Y, 18, 16, GFX.Font2(2).hdc, 2, C, vbSrcPaint
                frmMain.renderTexture(int(X + B), int(Y), 18, 16, GFX.Font2[2], 2, C, r, g, b, a);
//                B = B + 18
                B += 16;
//                If Left(Words, 1) = "M" Then B = B + 2
                if(c == 'M')
                    B += 2;
//            Else
            } else {
//                B = B + 16
                B += 16;
            }
//            End If
//            Words = Right(Words, Len(Words) - 1)
//        Loop
        }
//    ElseIf Font = 4 Then
    }
    else if(Font == 4)
    {
//        Do While Len(Words) > 0
        for(auto c : SuperWords)
        {
//            If Asc(Left(Words, 1)) >= 33 And Asc(Left(Words, 1)) <= 126 Then
            if(c >= 33 && c <= 126)
            {
//                C = (Asc(Left(Words, 1)) - 33) * 16
                C = (c - 33) * 16;
//                BitBlt myBackBuffer, X + B, Y, 18, 16, GFX.Font2(3).hdc, 2, C, vbSrcPaint
                frmMain.renderTexture(int(X + B), int(Y), 18, 16, GFX.Font2[3], 2, C, r, g, b, a);
//                B = B + 18
                B += 18;
//            Else
            } else {
//                B = B + 18
                B += 18;
//            End If
            }
//            Words = Right(Words, Len(Words) - 1)
//        Loop
        }
//    End If
    }
}

void SetRes()
{
    GetCurrentRes();
    resChanged = true;
    ChangeRes(800, 600, 16, 60); // 800x600 pixels, 16 bit Color, 60Hz
    ShowCursor(0);
}

std::string CheckKey(std::string newStrizzle)
{
    return newStrizzle; // DUMMY
//    CheckKey = newStrizzle
//    If newStrizzle = Chr(vbKeyUp) Then CheckKey = "UP"
//    If newStrizzle = Chr(vbKeyDown) Then CheckKey = "DOWN"
//    If newStrizzle = Chr(vbKeyLeft) Then CheckKey = "LEFT"
//    If newStrizzle = Chr(vbKeyRight) Then CheckKey = "RIGHT"
//    If newStrizzle = Chr(vbKeyEscape) Then CheckKey = "ESCAPE"
//    If newStrizzle = Chr(vbKeyShift) Then CheckKey = "SHIFT"
//    If newStrizzle = Chr(vbKeyReturn) Then CheckKey = "RETURN"
//    If newStrizzle = Chr(vbKeyControl) Then CheckKey = "CONTROL"
//    If newStrizzle = Chr(vbKeySpace) Then CheckKey = "SPACE"
//    If newStrizzle = Chr(vbKeyMenu) Then CheckKey = "ALT"
//    If newStrizzle = Chr(vbKeyF1) Then CheckKey = "F1"
//    If newStrizzle = Chr(vbKeyF2) Then CheckKey = "F2"
//    If newStrizzle = Chr(vbKeyF3) Then CheckKey = "F3"
//    If newStrizzle = Chr(vbKeyF4) Then CheckKey = "F4"
//    If newStrizzle = Chr(vbKeyF5) Then CheckKey = "F5"
//    If newStrizzle = Chr(vbKeyF6) Then CheckKey = "F6"
//    If newStrizzle = Chr(vbKeyF7) Then CheckKey = "F7"
//    If newStrizzle = Chr(vbKeyF8) Then CheckKey = "F8"
//    If newStrizzle = Chr(vbKeyF9) Then CheckKey = "F9"
//    If newStrizzle = Chr(vbKeyF10) Then CheckKey = "F10"
//    If newStrizzle = Chr(vbKeyF11) Then CheckKey = "F11"
//    If newStrizzle = Chr(vbKeyF12) Then CheckKey = "F12"
//    If newStrizzle = Chr(vbKeyBack) Then CheckKey = "BACKSPACE"
//    If newStrizzle = Chr(vbKeyTab) Then CheckKey = "TAB"
//    If newStrizzle = Chr(vbKeyInsert) Then CheckKey = "INSERT"
//    If newStrizzle = Chr(vbKeyDelete) Then CheckKey = "DELETE"
//    If newStrizzle = Chr(vbKeyHome) Then CheckKey = "HOME"
//    If newStrizzle = Chr(vbKeyEnd) Then CheckKey = "END"
//    If newStrizzle = Chr(vbKeyPageUp) Then CheckKey = "PAGE UP"
//    If newStrizzle = Chr(vbKeyPageDown) Then CheckKey = "PAGE DOWN"
}

/*Private*/
void SpecialFrames()
{
    SpecialFrameCount[1] = SpecialFrameCount[1] + 1;
    if(SpecialFrameCount[1] >= 6)
    {
        SpecialFrame[1] = SpecialFrame[1] + 1;
        if(SpecialFrame[1] >= 2)
            SpecialFrame[1] = 0;
        SpecialFrameCount[1] = 0;
    }
    SpecialFrameCount[2] = SpecialFrameCount[2] + 1;
    if(SpecialFrameCount[2] >= 3)
    {
        SpecialFrame[2] = SpecialFrame[2] + 1;
        if(SpecialFrame[2] >= 4)
            SpecialFrame[2] = 0;
        SpecialFrameCount[2] = 0;
    }
    SpecialFrameCount[3] = SpecialFrameCount[3] + 1;
    if(SpecialFrameCount[3] >= 8)
    {
        SpecialFrame[3] = SpecialFrame[3] + 1;
        if(SpecialFrame[3] >= 4)
            SpecialFrame[3] = 0;
        SpecialFrameCount[3] = 0;
    }
    SpecialFrameCount[4] = SpecialFrameCount[4] + 1;
    if(SpecialFrameCount[4] >= 2.475F)
    {
        SpecialFrame[4] = SpecialFrame[4] + 1;
        if(SpecialFrame[4] >= 4)
            SpecialFrame[4] = 0;
        SpecialFrameCount[4] = (float)(SpecialFrameCount[4] - 2.475);
    }
    SpecialFrameCount[5] = SpecialFrameCount[5] + 1;
    if(SpecialFrameCount[5] < 20)
        SpecialFrame[5] = 1;
    else if(SpecialFrameCount[5] < 25)
        SpecialFrame[5] = 2;
    else if(SpecialFrameCount[5] < 30)
        SpecialFrame[5] = 3;
    else if(SpecialFrameCount[5] < 35)
        SpecialFrame[5] = 4;
    else if(SpecialFrameCount[5] < 40)
        SpecialFrame[5] = 5;
    else if(SpecialFrameCount[5] < 45)
        SpecialFrame[5] = 1;
    else if(SpecialFrameCount[5] < 50)
        SpecialFrame[5] = 0;
    else
        SpecialFrameCount[5] = 0;
    SpecialFrameCount[6] = SpecialFrameCount[6] + 1;
    if(SpecialFrameCount[6] >= 12)
    {
        SpecialFrame[6] = SpecialFrame[6] + 1;
        if(SpecialFrame[6] >= 4)
            SpecialFrame[6] = 0;
        SpecialFrameCount[6] = 0;
    }
    SpecialFrameCount[7] = SpecialFrameCount[7] + 1;
    if(SpecialFrameCount[7] < 8)
        SpecialFrame[7] = 0;
    else if(SpecialFrameCount[7] < 16)
        SpecialFrame[7] = 1;
    else if(SpecialFrameCount[7] < 24)
        SpecialFrame[7] = 2;
    else if(SpecialFrameCount[7] < 32)
        SpecialFrame[7] = 3;
    else if(SpecialFrameCount[7] < 40)
        SpecialFrame[7] = 2;
    else if(SpecialFrameCount[7] < 48)
        SpecialFrame[7] = 1;
    else
        SpecialFrameCount[7] = 0;
    SpecialFrameCount[8] = SpecialFrameCount[8] + 1;
    if(SpecialFrameCount[8] >= 8)
    {
        SpecialFrame[8] = SpecialFrame[8] + 1;
        if(SpecialFrame[8] >= 3)
            SpecialFrame[8] = 0;
        SpecialFrameCount[8] = 0;
    }

    SpecialFrameCount[9] = SpecialFrameCount[9] + 1; // Fairy frame
    if(SpecialFrameCount[9] >= 8)
    {
        SpecialFrame[9] = SpecialFrame[9] + 1;
        if(SpecialFrame[9] >= 2)
            SpecialFrame[9] = 0;
        SpecialFrameCount[9] = 0;
    }
}

void DrawBackground(int S, int Z)
{
    int A = 0;
    int B = 0;
    Location_t tempLocation;
    Location_t tempLevel;
    tempLevel = level[S];
    if(LevelEditor == true)
        LevelREAL[S] = level[S];
    level[S] = LevelREAL[S];
    A = 1; // Blocks
    if(Background2[S] == 1)
    {
        int tempVar = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 2; // Clouds
    if(Background2[S] == 1 || Background2[S] == 2 || Background2[S] == 22)
    {
        int tempVar2 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = 0; B <= tempVar2; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A] - ScreenH + 100;
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    if(Background2[S] == 13)
    {

        int tempVar3 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = 0; B <= tempVar3; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 3; // Hills
    if(Background2[S] == 2)
    {

        int tempVar4 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar4; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 4; // Castle
    if(Background2[S] == 3)
    {

        int tempVar5 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = 0; B <= tempVar5; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 5; // Pipes
    if(Background2[S] == 4)
    {

        int tempVar6 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar6; B++)
        {
            tempLocation.Y = level[S].Y + ((B * GFXBackground2Height[A] - B) - (vScreenY[Z] + vScreen[Z].Top + level[S].Y) * 0.5) - 32;
            if(level[S].Width - level[S].X > GFXBackground2Width[A])
            {
                tempLocation.X = (-vScreenX[Z] - level[S].X) / (level[S].Width - level[S].X - (800 - vScreen[Z].Left)) * (GFXBackground2Width[A] - (800 - vScreen[Z].Left));
                tempLocation.X = -vScreenX[Z] - tempLocation.X;
            }
            else
                tempLocation.X = level[S].Width - GFXBackground2Width[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 6; // Trees
    if(Background2[S] == 5)
    {

        int tempVar7 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar7; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 7; // Bonus
    if(Background2[S] == 6)
    {

        int tempVar8 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar8; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 8; // SMB Underground
    if(Background2[S] == 7)
    {

        int tempVar9 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar9; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Y - 32;
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 9; // Night
    if(Background2[S] == 8 || Background2[S] == 9)
    {

        int tempVar10 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = 0; B <= tempVar10; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 10; // Night 2
    if(Background2[S] == 9)
    {

        int tempVar11 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar11; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 11; // Overworld
    if(Background2[S] == 10)
    {

        int tempVar12 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar12; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 12; // SMW Hills
    if(Background2[S] == 11)
    {

        int tempVar13 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar13; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 13; // SMW Trees
    if(Background2[S] == 12)
    {

        int tempVar14 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar14; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 14; // SMB3 Desert
    if(Background2[S] == 14)
    {

        int tempVar15 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar15; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 15; // SMB3 Dungeon
    if(Background2[S] == 15)
    {

        int tempVar16 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar16; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 16; // Crateria
    if(Background2[S] == 16)
    {

        int tempVar17 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar17; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 17; // smb3 ship
    if(Background2[S] == 17)
    {

        int tempVar18 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar18; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Y - 32;
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 18; // SMW ghost house
    if(Background2[S] == 18)
    {

        int tempVar19 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar19; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 4.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }
        }
    }
    A = 19; // smw forest
    if(Background2[S] == 19)
    {

        int tempVar20 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar20; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 20; // smb3 forest
    if(Background2[S] == 20)
    {

        int tempVar21 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar21; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 21; // smb3 battle game
    if(Background2[S] == 21)
    {

        int tempVar22 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar22; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 22; // SMB3 Waterfall
    if(Background2[S] == 22)
    {

        int tempVar23 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar23; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }
        }
    }
    A = 23; // SMB3 Tank
    if(Background2[S] == 23)
    {

        int tempVar24 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar24; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 24; // smb3 bowsers castle
    if(Background2[S] == 24)
    {

        int tempVar25 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar25; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Y - 32; // Height - GFXBackground2height(a)
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 25; // SMB2 Underground
    if(Background2[S] == 25)
    {

        int tempVar26 = static_cast<int>(floor(static_cast<double>((level[S].Height - level[S].Y) / GFXBackground2Height[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar26; B++)
        {
            tempLocation.Y = level[S].Y + ((B * GFXBackground2Height[A] - B) - (vScreenY[Z] + vScreen[Z].Top + level[S].Y) * 0.5) - 32;
            // .X = Level(S).X
            if(level[S].Width - level[S].X > GFXBackground2Width[A])
            {
                // .X = (-vScreenX(Z) - level(S).X) / (level(S).Width - level(S).X - 800) * (GFXBackground2Width(A) - 800)
                tempLocation.X = (-vScreenX[Z] - level[S].X) / (level[S].Width - level[S].X - (800 - vScreen[Z].Left)) * (GFXBackground2Width[A] - (800 - vScreen[Z].Left));
                tempLocation.X = -vScreenX[Z] - tempLocation.X;
            }
            else
                tempLocation.X = level[S].Width - GFXBackground2Width[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 26; // Toad's House
    if(Background2[S] == 26)
    {

        int tempVar27 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar27; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Y - 32; // Height - GFXBackground2height(a)
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 27; // SMB3 Castle
    if(Background2[S] == 27)
    {

        int tempVar28 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar28; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 28; // SMW Bonus
    if(Background2[S] == 28)
    {

        int tempVar29 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 3;
        for(B = 0; B <= tempVar29; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 29; // SMW Night
    if(Background2[S] == 29)
    {

        int tempVar30 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar30; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 4.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[6]);
            }
        }
    }
    A = 30; // SMW Cave
    if(Background2[S] == 30)
    {

        int tempVar31 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar31; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 4.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];

            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }


        }
    }
    A = 31; // SMW Hills 2
    if(Background2[S] == 31)
    {

        int tempVar32 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar32; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 32; // SMW Clouds
    if(Background2[S] == 32)
    {

        int tempVar33 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar33; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 33; // SMW Snow
    if(Background2[S] == 33)
    {

        int tempVar34 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar34; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 34; // SMW Hills 3
    if(Background2[S] == 34)
    {

        int tempVar35 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar35; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 35; // SMB 3 Snow Trees
    if(Background2[S] == 35)
    {

        int tempVar36 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar36; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 36; // Snow Clouds
    if(Background2[S] == 35 || Background2[S] == 37)
    {

        int tempVar37 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = 0; B <= tempVar37; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A] - ScreenH + 100;
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    if(Background2[S] == 36)
    {

        int tempVar38 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = 0; B <= tempVar38; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 37; // SMB 3 Snow Hills
    if(Background2[S] == 37)
    {

        int tempVar39 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar39; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 38; // SMB3 Cave with Sky
    if(Background2[S] == 38)
    {

        int tempVar40 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar40; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            tempLocation.Y = level[S].Y - 20; // Height - GFXBackground2height(a)
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 39; // SMB3 Cave no Sky
    if(Background2[S] == 39)
    {

        int tempVar41 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar41; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 40; // Mystic Cave Zone
    if(Background2[S] == 40)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {



            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (ScreenH - vScreen(Z).Top)) * (GFXBackground2Height(A) - (ScreenH - vScreen(Z).Top))
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)



            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar42 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar42; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 953, GFXBackground2Width[A], 47, GFXBackground2[A], 0, 953);
            }
        }

        int tempVar43 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6))) + 1;
        for(B = 0; B <= tempVar43; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 916, GFXBackground2Width[A], 37, GFXBackground2[A], 0, 916);
            }
        }

        int tempVar44 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.7))) + 1;
        for(B = 0; B <= tempVar44; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.7);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 849, GFXBackground2Width[A], 67, GFXBackground2[A], 0, 849);
            }
        }

        int tempVar45 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8))) + 1;
        for(B = 0; B <= tempVar45; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 815, GFXBackground2Width[A], 34, GFXBackground2[A], 0, 815);
            }
        }

        int tempVar46 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9))) + 1;
        for(B = 0; B <= tempVar46; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 709, GFXBackground2Width[A], 106, GFXBackground2[A], 0, 709);
            }
        }

        int tempVar47 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85))) + 1;
        for(B = 0; B <= tempVar47; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 664, GFXBackground2Width[A], 45, GFXBackground2[A], 0, 664);
            }
        }

        int tempVar48 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8))) + 1;
        for(B = 0; B <= tempVar48; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 614, GFXBackground2Width[A], 50, GFXBackground2[A], 0, 614);
            }
        }

        int tempVar49 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = 0; B <= tempVar49; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 540, GFXBackground2Width[A], 74, GFXBackground2[A], 0, 540);
            }
        }

        int tempVar50 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.7))) + 1;
        for(B = 0; B <= tempVar50; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.7);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 408, GFXBackground2Width[A], 132, GFXBackground2[A], 0, 408);
            }
        }

        int tempVar51 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = 0; B <= tempVar51; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 333, GFXBackground2Width[A], 75, GFXBackground2[A], 0, 333);
            }
        }

        int tempVar52 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8))) + 1;
        for(B = 0; B <= tempVar52; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 278, GFXBackground2Width[A], 55, GFXBackground2[A], 0, 278);
            }
        }

        int tempVar53 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85))) + 1;
        for(B = 0; B <= tempVar53; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 235, GFXBackground2Width[A], 43, GFXBackground2[A], 0, 235);
            }
        }

        int tempVar54 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9))) + 1;
        for(B = 0; B <= tempVar54; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 123, GFXBackground2Width[A], 112, GFXBackground2[A], 0, 123);
            }
        }

        int tempVar55 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.8))) + 1;
        for(B = 0; B <= tempVar55; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.8);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 85, GFXBackground2Width[A], 38, GFXBackground2[A], 0, 85);
            }
        }

        int tempVar56 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.7))) + 1;
        for(B = 0; B <= tempVar56; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.7);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 48, GFXBackground2Width[A], 37, GFXBackground2[A], 0, 48);
            }
        }

        int tempVar57 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6))) + 1;
        for(B = 0; B <= tempVar57; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 41; // SMB 1 Castle
    if(Background2[S] == 41)
    {

        int tempVar58 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = 0; B <= tempVar58; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 42; // SMW Castle
    if(Background2[S] == 42)
    {

        int tempVar59 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar59; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {

                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y

                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 4.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }
        }
    }
    A = 43; // SMW Castle 2
    if(Background2[S] == 43)
    {

        int tempVar60 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar60; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 44; // SMB2 Castle
    if(Background2[S] == 44)
    {

        int tempVar61 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar61; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                tempLocation.Y = (-vScreenY[Z] - level[S].Y) / (level[S].Height - level[S].Y - 600) * (GFXBackground2Height[A] - 600);
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 45; // Brinstar
    if(Background2[S] == 45)
    {

        int tempVar62 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = 0; B <= tempVar62; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 46; // Transport
    if(Background2[S] == 46)
    {

        int tempVar63 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = 0; B <= tempVar63; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 47; // Transport
    if(Background2[S] == 47)
    {

        int tempVar64 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = 0; B <= tempVar64; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 48; // SMB2 Blouds
    if(Background2[S] == 48)
    {

        int tempVar65 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = 0; B <= tempVar65; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 49; // Desert Night
    if(Background2[S] == 49)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar66 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar66; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 280, GFXBackground2Width[A], 450, GFXBackground2[A], 0, 280);
            }
        }

        int tempVar67 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9))) + 1;
        for(B = 0; B <= tempVar67; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 268, GFXBackground2Width[A], 12, GFXBackground2[A], 0, 268);
            }
        }

        int tempVar68 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.89))) + 1;
        for(B = 0; B <= tempVar68; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.89);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 244, GFXBackground2Width[A], 24, GFXBackground2[A], 0, 244);
            }
        }

        int tempVar69 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.88))) + 1;
        for(B = 0; B <= tempVar69; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.88);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 228, GFXBackground2Width[A], 16, GFXBackground2[A], 0, 228);
            }
        }

        int tempVar70 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.87))) + 1;
        for(B = 0; B <= tempVar70; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.87);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 196, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 196);
            }
        }

        int tempVar71 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.86))) + 1;
        for(B = 0; B <= tempVar71; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.86);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 164, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 164);
            }
        }

        int tempVar72 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85))) + 1;
        for(B = 0; B <= tempVar72; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 116, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 116);
            }
        }

        int tempVar73 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.84))) + 1;
        for(B = 0; B <= tempVar73; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.84);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 58, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 58);
            }
        }

        int tempVar74 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.83))) + 1;
        for(B = 0; B <= tempVar74; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.83);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 50; // Shrooms
    if(Background2[S] == 50)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar75 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar75; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 378, GFXBackground2Width[A], 378, GFXBackground2[A], 0, 378);
            }
        }

        int tempVar76 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.65))) + 1;
        for(B = 0; B <= tempVar76; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.65);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 220, GFXBackground2[A], 0, 0);
            }
        }

        int tempVar77 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6))) + 1;
        for(B = 0; B <= tempVar77; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 220, GFXBackground2Width[A], 159, GFXBackground2[A], 0, 220);
            }
        }
    }

    A = 51; // SMB1 Desert
    if(Background2[S] == 51)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar78 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.75))) + 1;
        for(B = 0; B <= tempVar78; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.75);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 350, GFXBackground2[A], 0, 0);
            }
        }

        int tempVar79 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar79; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 350, GFXBackground2Width[A], GFXBackground2Height[A] - 350, GFXBackground2[A], 0, 350);
            }
        }
    }
    A = 52; // SMB2 Desert Night
    if(Background2[S] == 52)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];

        int tempVar80 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar80; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 280, GFXBackground2Width[A], GFXBackground2Height[A] - 280, GFXBackground2[A], 0, 280);
            }
        }

        int tempVar81 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.9))) + 1;
        for(B = 0; B <= tempVar81; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.9);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 268, GFXBackground2Width[A], 12, GFXBackground2[A], 0, 268);
            }
        }

        int tempVar82 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.89))) + 1;
        for(B = 0; B <= tempVar82; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.89);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 244, GFXBackground2Width[A], 24, GFXBackground2[A], 0, 244);
            }
        }

        int tempVar83 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.88))) + 1;
        for(B = 0; B <= tempVar83; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.88);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 228, GFXBackground2Width[A], 16, GFXBackground2[A], 0, 228);
            }
        }

        int tempVar84 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.87))) + 1;
        for(B = 0; B <= tempVar84; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.87);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 196, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 196);
            }
        }

        int tempVar85 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.86))) + 1;
        for(B = 0; B <= tempVar85; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.86);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 164, GFXBackground2Width[A], 32, GFXBackground2[A], 0, 164);
            }
        }

        int tempVar86 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.85))) + 1;
        for(B = 0; B <= tempVar86; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.85);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 116, GFXBackground2Width[A], 48, GFXBackground2[A], 0, 116);
            }
        }

        int tempVar87 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.84))) + 1;
        for(B = 0; B <= tempVar87; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.84);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 58, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 58);
            }
        }

        int tempVar88 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.83))) + 1;
        for(B = 0; B <= tempVar88; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.83);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 58, GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 53; // Cliffs
    if(Background2[S] == 53)
    {

        int tempVar89 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = 0; B <= tempVar89; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 54; // Warehouse
    if(Background2[S] == 54)
    {

        int tempVar90 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = 0; B <= tempVar90; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 55; // SMW Water
    if(Background2[S] == 55)
    {

        int tempVar91 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar91; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) / 4 - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 4.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[3]);
            }
        }
    }
    A = 56; // SMB3 Water
    if(Background2[S] == 56)
    {
        if(level[S].Height - level[S].Y > GFXBackground2Height[A])
        {
            // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
            tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
            tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
        }
        else
            tempLocation.Y = level[S].Height - GFXBackground2Height[A];
        tempLocation.Height = GFXBackground2Height[A];
        tempLocation.Width = GFXBackground2Width[A];



        int tempVar92 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.65))) + 1;
        for(B = 0; B <= tempVar92; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.65);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], 100, GFXBackground2[A], 0, 0);
            }
        }


        int tempVar93 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.6))) + 1;
        for(B = 0; B <= tempVar93; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.6);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 100, GFXBackground2Width[A], 245, GFXBackground2[A], 0, 100);
            }
        }


        int tempVar94 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.55))) + 1;
        for(B = 0; B <= tempVar94; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.55);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 345, GFXBackground2Width[A], 110, GFXBackground2[A], 0, 345);
            }
        }


        int tempVar95 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar95; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y + 455, GFXBackground2Width[A], GFXBackground2Height[A] - 455, GFXBackground2[A], 0, 455);
            }
        }
    }
    A = 57; // Warehouse
    if(Background2[S] == 57)
    {

        int tempVar96 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 2;
        for(B = 0; B <= tempVar96; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A])
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - 600) * (GFXBackground2Height(A) - 600)
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A];
            tempLocation.Height = GFXBackground2Height[A];
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A], GFXBackground2[A], 0, 0);
            }
        }
    }
    A = 58; // SMW Night
    if(Background2[S] == 58)
    {

        int tempVar97 = static_cast<int>(floor(static_cast<double>((level[S].Width - level[S].X) / GFXBackground2Width[A] * 0.5))) + 1;
        for(B = 0; B <= tempVar97; B++)
        {
            tempLocation.X = level[S].X + ((B * GFXBackground2Width[A]) - (vScreenX[Z] + vScreen[Z].Left + level[S].X) * 0.5);
            if(level[S].Height - level[S].Y > GFXBackground2Height[A] / 4.0)
            {
                // .Y = (-vScreenY(Z) - level(S).Y) / (level(S).Height - level(S).Y - (600 - vScreen(Z).Top)) * (GFXBackground2Height(A) / 4 - (600 - vScreen(Z).Top))
                // .Y = -vScreenY(Z) - .Y
                tempLocation.Y = (-vScreenY[Z] - vScreen[Z].Top - level[S].Y) / (level[S].Height - level[S].Y - ScreenH) * (GFXBackground2Height[A] / 4.0 - ScreenH) + vScreen[Z].Top;
                tempLocation.Y = -vScreenY[Z] - tempLocation.Y;
            }
            else
                tempLocation.Y = level[S].Height - GFXBackground2Height[A] / 4.0;
            tempLocation.Height = GFXBackground2Height[A] / 4;
            tempLocation.Width = GFXBackground2Width[A];
            if(vScreenCollision(Z, tempLocation))
            {
                frmMain.renderTexture(vScreenX[Z] + tempLocation.X, vScreenY[Z] + tempLocation.Y, GFXBackground2Width[A], GFXBackground2Height[A] / 4.0, GFXBackground2[A], 0, (GFXBackground2Height[A] / 4.0) * SpecialFrame[6]);
            }
        }
    }
    level[S] = tempLevel;

}

void PlayerWarpGFX(int A, Location_t tempLocation, float X2, float Y2)
{
    // .Effect = 3      -- Warp Pipe
    // .Effect2 = 0     -- Entering
    // .Effect2 = 1     -- Move to next spot
    // .Effect2 => 100  -- Delay at next spot
    // .Effect2 = 2     -- Exiting
    // .Effect2 = 3     -- Done
    if(Player[A].Effect2 == 0)
    {
        if(Warp[Player[A].Warp].Direction == 3) // Moving down
        {
            if(tempLocation.Height > (Warp[Player[A].Warp].Entrance.Y + Warp[Player[A].Warp].Entrance.Height) - (tempLocation.Y))
                tempLocation.Height = (Warp[Player[A].Warp].Entrance.Y + Warp[Player[A].Warp].Entrance.Height) - (tempLocation.Y);
        }
        else if(Warp[Player[A].Warp].Direction == 1) // Moving up
        {
            if(Warp[Player[A].Warp].Entrance.Y > tempLocation.Y)
            {
                Y2 = (Warp[Player[A].Warp].Entrance.Y - tempLocation.Y);
                tempLocation.Y = Warp[Player[A].Warp].Entrance.Y;
                tempLocation.Height = tempLocation.Height - Y2;
            }
        }
        else if(Warp[Player[A].Warp].Direction == 4) // Moving right
            tempLocation.Width = (Warp[Player[A].Warp].Entrance.X + Warp[Player[A].Warp].Entrance.Width) - (tempLocation.X);
        else if(Warp[Player[A].Warp].Direction == 2) // Moving left
        {
            X2 = (Warp[Player[A].Warp].Entrance.X - tempLocation.X);
            if(X2 < 0)
                X2 = 0;
            else
                tempLocation.X = Warp[Player[A].Warp].Entrance.X;
        }
    }
    else if(Player[A].Effect2 == 2)
    {
        if(Warp[Player[A].Warp].Direction2 == 3) // Moving up
        {
            if(tempLocation.Height > (Warp[Player[A].Warp].Exit.Y + Warp[Player[A].Warp].Exit.Height) - (tempLocation.Y))
                tempLocation.Height = (Warp[Player[A].Warp].Exit.Y + Warp[Player[A].Warp].Exit.Height) - (tempLocation.Y);
        }
        else if(Warp[Player[A].Warp].Direction2 == 1) // Moving down
        {
            if(Warp[Player[A].Warp].Exit.Y > tempLocation.Y)
            {
                Y2 = (Warp[Player[A].Warp].Exit.Y - tempLocation.Y);
                tempLocation.Y = Warp[Player[A].Warp].Exit.Y;
                tempLocation.Height = tempLocation.Height - Y2;
            }
        }
        else if(Warp[Player[A].Warp].Direction2 == 4) // Moving left
            tempLocation.Width = (Warp[Player[A].Warp].Exit.X + Warp[Player[A].Warp].Exit.Width) - (tempLocation.X);
        else if(Warp[Player[A].Warp].Direction2 == 2) // Moving right
        {
            X2 = (Warp[Player[A].Warp].Exit.X - tempLocation.X);
            if(X2 < 0)
                X2 = 0;
            else
                tempLocation.X = Warp[Player[A].Warp].Exit.X;
        }
    }
    if(Player[A].Effect2 == 1 || Player[A].Effect2 >= 100)
        tempLocation.Height = 0;
    if(tempLocation.Height < 0)
    {
        tempLocation.Height = 0;
        tempLocation.Width = 0;
    }
    tempLocation.Width = tempLocation.Width - X2;
}

void NPCWarpGFX(int A, Location_t tempLocation, float X2, float Y2)
{
    // player(a).effect = 3      -- Warp Pipe
    // player(a).effect2 = 0     -- Entering
    // player(a).effect2 = 1     -- Move to next spot
    // player(a).effect2 => 100  -- Delay at next spot
    // player(a).effect2 = 2     -- Exiting
    // player(a).effect2 = 3     -- Done
    if(Player[A].Effect2 == 0)
    {
        if(Warp[Player[A].Warp].Direction == 3) // Moving down
        {
            if(tempLocation.Height > (Warp[Player[A].Warp].Entrance.Y + Warp[Player[A].Warp].Entrance.Height) - (tempLocation.Y))
                tempLocation.Height = (Warp[Player[A].Warp].Entrance.Y + Warp[Player[A].Warp].Entrance.Height) - (tempLocation.Y);
        }
        else if(Warp[Player[A].Warp].Direction == 1) // Moving up
        {
            if(Warp[Player[A].Warp].Entrance.Y > tempLocation.Y)
            {
                Y2 = (Warp[Player[A].Warp].Entrance.Y - tempLocation.Y);
                tempLocation.Y = Warp[Player[A].Warp].Entrance.Y;
                tempLocation.Height = tempLocation.Height - Y2;
            }
        }
        else if(Warp[Player[A].Warp].Direction == 4) // Moving right
            tempLocation.Width = (Warp[Player[A].Warp].Entrance.X + Warp[Player[A].Warp].Entrance.Width) - (tempLocation.X);
        else if(Warp[Player[A].Warp].Direction == 2) // Moving left
        {
            X2 = (Warp[Player[A].Warp].Entrance.X - tempLocation.X);
            if(X2 < 0)
                X2 = 0;
            else
                tempLocation.X = Warp[Player[A].Warp].Entrance.X;
        }
    }
    else if(Player[A].Effect2 == 2)
    {
        if(Warp[Player[A].Warp].Direction2 == 3) // Moving up
        {
            if(tempLocation.Height > (Warp[Player[A].Warp].Exit.Y + Warp[Player[A].Warp].Exit.Height) - (tempLocation.Y))
                tempLocation.Height = (Warp[Player[A].Warp].Exit.Y + Warp[Player[A].Warp].Exit.Height) - (tempLocation.Y);
        }
        else if(Warp[Player[A].Warp].Direction2 == 1) // Moving down
        {
            if(Warp[Player[A].Warp].Exit.Y > tempLocation.Y)
            {
                Y2 = (Warp[Player[A].Warp].Exit.Y - tempLocation.Y);
                tempLocation.Y = Warp[Player[A].Warp].Exit.Y;
                tempLocation.Height = tempLocation.Height - Y2;
            }
        }
        else if(Warp[Player[A].Warp].Direction2 == 4) // Moving left
            tempLocation.Width = (Warp[Player[A].Warp].Exit.X + Warp[Player[A].Warp].Exit.Width) - (tempLocation.X);
        else if(Warp[Player[A].Warp].Direction2 == 2) // Moving right
        {
            X2 = (Warp[Player[A].Warp].Exit.X - tempLocation.X);
            if(X2 < 0)
                X2 = 0;
            else
                tempLocation.X = Warp[Player[A].Warp].Exit.X;
        }
    }
    if(Player[A].Effect2 == 1 || Player[A].Effect2 >= 100)
        tempLocation.Height = 0;
    if(tempLocation.Height < 0)
    {
        tempLocation.Height = 0;
        tempLocation.Width = 0;
    }
}

// change from fullscreen to windowed mode
void ChangeScreen()
{
//    frmMain.LockSize = True
//    If resChanged = True Then
    if(resChanged)
    {
//        SetOrigRes
        SetOrigRes();
//        DoEvents
        DoEvents();
//        DeleteDC myBackBuffer
//        DeleteObject myBufferBMP
//        DoEvents
//        myBackBuffer = CreateCompatibleDC(frmMain.hdc)
//        myBufferBMP = CreateCompatibleBitmap(frmMain.hdc, 800, 600)
//        SelectObject myBackBuffer, myBufferBMP
//        frmMain.BorderStyle = 2
//        frmMain.Caption = "Super Mario Bros. X - Version 1.3 - www.SuperMarioBrothers.org"
//        frmMain.Left = 0
//        frmMain.Top = 0
//    Else
    } else {
//        If frmMain.WindowState = 2 Then
//            frmMain.WindowState = 0
//        End If
//        frmMain.Width = 12240
//        frmMain.Height = 9570
//        Do While frmMain.ScaleWidth > 800
//            frmMain.Width = frmMain.Width - 5
//        Loop
//        Do While frmMain.ScaleHeight > 600
//            frmMain.Height = frmMain.Height - 5
//        Loop
//        Do While frmMain.ScaleWidth < 800
//            frmMain.Width = frmMain.Width + 5
//        Loop
//        Do While frmMain.ScaleHeight < 600
//            frmMain.Height = frmMain.Height + 5
//        Loop
//        SetRes
        SetRes();
//        DoEvents
        DoEvents();
//        DeleteDC myBackBuffer
//        DeleteObject myBufferBMP
//        DoEvents
//        myBackBuffer = CreateCompatibleDC(frmMain.hdc)
//        myBufferBMP = CreateCompatibleBitmap(frmMain.hdc, 800, 600)
//        SelectObject myBackBuffer, myBufferBMP
//        frmMain.BorderStyle = 0
//        frmMain.Caption = ""
//        frmMain.Left = 0
//        frmMain.Top = 0
//    End If
    }
//    SaveConfig
    SaveConfig();
//    frmMain.LockSize = False
}

void GetvScreenCredits()
{
    int A = 0;
    int B = 0;
    vScreenX[1] = 0;
    vScreenY[1] = 0;
    for(A = 1; A <= numPlayers; A++)
    {
        if(Player[A].Dead == false && Player[A].Effect != 6)
        {
            vScreenX[1] = vScreenX[1] - Player[A].Location.X - Player[A].Location.Width / 2.0;
            if(Player[A].Mount == 2)
                vScreenY[1] = vScreenY[1] - Player[A].Location.Y;
            else
                vScreenY[1] = vScreenY[1] - Player[A].Location.Y - Player[A].Location.Height;
            B = B + 1;
        }
    }
    A = 1;
    if(B == 0)
        return;
    vScreenX[1] = (vScreenX[1] / B) + (ScreenW * 0.5);
    vScreenY[1] = (vScreenY[1] / B) + (ScreenH * 0.5) - vScreenYOffset;
    if(-vScreenX[A] < level[Player[1].Section].X)
        vScreenX[A] = -level[Player[1].Section].X;
    if(-vScreenX[A] + ScreenW > level[Player[1].Section].Width)
        vScreenX[A] = -(level[Player[1].Section].Width - ScreenW);
    if(-vScreenY[A] < level[Player[1].Section].Y + 100)
        vScreenY[A] = -level[Player[1].Section].Y + 100;
    if(-vScreenY[A] + ScreenH > level[Player[1].Section].Height - 100)
        vScreenY[A] = -(level[Player[1].Section].Height - ScreenH) - 100;
}

void DoCredits()
{
    int A = 0;
    if(GameMenu == true)
        return;

    for(A = 1; A <= numCredits; A++)
    {
        Credit[A].Location.Y = Credit[A].Location.Y - 0.8;

        if(Credit[A].Location.Y <= 600 && Credit[A].Location.Y + Credit[A].Location.Height >= 0)
        {
            SuperPrint(Credit[A].Text, 4, static_cast<float>(Credit[A].Location.X), static_cast<float>(Credit[A].Location.Y));
        }

        else if(A == numCredits && Credit[A].Location.Y + Credit[A].Location.Height < -100)
        {
            CreditChop = (float)(CreditChop + 0.4f);
            if(CreditChop >= 300)
            {
                CreditChop = 300;
                EndCredits = EndCredits + 1;
                if(EndCredits == 300)
                {
                    SetupCredits();
                    GameOutro = false;
                    GameMenu = true;
                }
            }
            else
                EndCredits = 0;
        }
        else if(CreditChop > 100 && Credit[numCredits].Location.Y + Credit[numCredits].Location.Height > 0)
        {
            CreditChop = (float)(CreditChop - 0.02f);
            if(CreditChop < 100)
                CreditChop = 100;
        }
        else if(musicPlaying == false)
        {
            musicName = "tmusic";
            // mciSendString "play tmusic from 10", 0, 0, 0
            PlayMusic("tmusic");
            musicPlaying = true;
        }
    }
    if(CreditChop <= 100 || EndCredits > 0)
    {
        for(A = 1; A <= 2; A++)
        {
            if(((getKeyState(vbKeyEscape) == KEY_PRESSED) | (getKeyState(vbKeySpace) == KEY_PRESSED) | (getKeyState(vbKeyReturn) == KEY_PRESSED)) != 0)
            {
                CreditChop = 300;
                EndCredits = 0;
                SetupCredits();
                GameMenu = true;
                GameOutro = false;
            }
        }
    }
}

void DrawInterface(int Z, int numScreens)
{
    int B = 0;
    int C = 0;
    int D = 0;

    std::string scoreStr = std::to_string(Score);
    std::string coinsStr = std::to_string(Coins);
    std::string livesStr = std::to_string(Lives);
    std::string numStarsStr = std::to_string(numStars);

    if(ScreenType == 5 || ScreenType == 6) // 2 Players
    {
        if(static_cast<int>(numScreens) == 1 && ScreenType != 6) // Only 1 screen
        {
            for(B = 1; B <= numPlayers; B++)
            {
                if(B == 1)
                    C = -40;
                else
                    C = 40;
                if(Player[B].Character == 3 || Player[B].Character == 4 || Player[B].Character == 5)
                {
                    if(B == 1)
                        D = -1;
                    else
                        D = 1;

                    if(Player[B].Hearts > 0)
                    {
                        frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32 + 17 * D, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32 + 17 * D, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                    }
                    if(Player[B].Hearts > 1)
                    {
                        frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 17 * D, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 17 * D, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                    }
                    if(Player[B].Hearts > 2)
                    {
                        frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32 + 17 * D, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                    }
                    else
                    {
                        frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32 + 17 * D, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                    }
                }
                else
                {
// 2 players 1 screen heldbonus
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C, 16, GFX.Container[1].w, GFX.Container[1].h, GFX.Container[Player[B].Character], 0, 0);
                    if(Player[B].HeldBonus > 0)
                    {
                        frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C + 12, 16 + 12, NPCWidth[Player[B].HeldBonus], NPCHeight[Player[B].HeldBonus], GFXNPC[Player[B].HeldBonus], 0, 0);
                    }
                }
            }

            for(B = 1; B <= 2; B++)
            {
                if(B == 1)
                    C = -58;
                else
                    C = 56;

                if(Player[B].Character == 5 && Player[B].Bombs > 0)
                {
                    frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 34 + C, 52, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[8], 0, 0);
                    frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 10 + C, 53, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(std::to_string(Player[B].Bombs), 1, 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12 + C, 53);
                }
            }
            C = 40;

            if(BattleMode == false)
            {
                // Print coins on the screen
                if((Player[1].HasKey | Player[2].HasKey) != 0)
                {
                    frmMain.renderTexture(-24 + 40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[0], 0, 0);
                }
                if(Player[1].Character == 5)
                {
                    frmMain.renderTexture(40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[6], 0, 0);
                }
                else
                {
                    frmMain.renderTexture(40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[2], 0, 0);
                }

                frmMain.renderTexture(40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

                SuperPrint(coinsStr, 1,
                           40 + 20 - (coinsStr.size() - 1) * 18 +
                           float(vScreen[Z].Width) / 2.0f - GFX.Container[1].w / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w,
                           16 + 11);
                // Print Score
                SuperPrint(scoreStr, 1,
                           40 + 20 - (int(scoreStr.size()) - 1) * 18 +
                           float(vScreen[Z].Width) / 2.0f - GFX.Container[1].w / 2 + 80 + 12 + 4 + 18 + 32 + GFX.Interface[3].w,
                           16 + 31);
                // Print lives on the screen
                frmMain.renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 - 16, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
                frmMain.renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(livesStr, 1, -80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w, 16 + 11);
                // Print stars on the screen
                if(numStars > 0)
                {

                    frmMain.renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122, 16 + 30, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                    frmMain.renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 31, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(numStarsStr, 1, -80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w, 16 + 31);
                }
            }
            else
            {
            // plr 1 score

                    frmMain.renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 - 16, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
                    frmMain.renderTexture(-80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(std::to_string(BattleLives[1]), 1, -80 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w, 16 + 11);
            // plr 2 score
                    frmMain.renderTexture(40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96 - 16, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[7], 0, 0);
                    frmMain.renderTexture(40 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(std::to_string(BattleLives[2]), 1, 24 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w, 16 + 11);
            }
        }
        else // Split screen
        {


// 2 players 2 screen heldbonus

            if(Player[Z].Character == 3 || Player[Z].Character == 4 || Player[Z].Character == 5)
            {
                if(Player[Z].Hearts > 0)
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }
                if(Player[Z].Hearts > 1)
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }
                if(Player[Z].Hearts > 2)
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }
            }
            else
            {

                frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2, 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX.Container[Player[Z].Character], 0, 0);
                if(Player[Z].HeldBonus > 0)
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12, 16 + 12, NPCWidth[Player[Z].HeldBonus], NPCHeight[Player[Z].HeldBonus], GFXNPC[Player[Z].HeldBonus], 0, 0);
                }
            }
            if(Player[Z].Character == 5 && Player[Z].Bombs > 0)
            {

                frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 34, 52, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[8], 0, 0);
                frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 10, 53, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(std::to_string(Player[Z].Bombs), 1, 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12, 53);
            }
            if(BattleMode == false)
            {
                // Print coins on the screen
                if(Player[Z].HasKey == true)
                {
                    frmMain.renderTexture(-24 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[0], 0, 0);
                }
                if(Player[Z].Character == 5)
                {
                    frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[6], 0, 0);
                }
                else
                {
                    frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[2], 0, 0);
                }


                frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(coinsStr, 1, 20 - (coinsStr.size() - 1) * 18 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w, 16 + 11);
                // Print Score
                SuperPrint(scoreStr, 1, 20 - (scoreStr.size() - 1) * 18 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 80 + 12 + 4 + 18 + 32 + GFX.Interface[3].w, 16 + 31);
                // Print lives on the screen

                frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 - 16, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
                frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(livesStr, 1, vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w, 16 + 11);
                // Print stars on the screen
                if(numStars > 0)
                {

                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122, 16 + 30, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);

                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 31, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                    SuperPrint(numStarsStr, 1, vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w, 16 + 31);
                }
            }
            else
            {
                if(Z == 1)
                {

                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
                }
                else
                {

                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[7], 0, 0);
                }

                frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w + 16, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                SuperPrint(std::to_string(BattleLives[Z]), 1, vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w + 16, 16 + 11);
            }
        }
    }
    else // 1 Player or Multi Mario
    {


//        if(nPlay.Online == false)
        {
            if(Player[1].Character == 3 || Player[1].Character == 4 || Player[1].Character == 5)
            {
//                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
//                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
//                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
                if(Player[1].Hearts > 0)
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }
                if(Player[1].Hearts > 1)
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }
                if(Player[1].Hearts > 2)
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
                }
                else
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
                }
            }
            else
            {
                frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2, 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX.Container[0], 0, 0);
                if(Player[1].HeldBonus > 0)
                {
                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12, 16 + 12, NPCWidth[Player[1].HeldBonus], NPCHeight[Player[1].HeldBonus], GFXNPC[Player[1].HeldBonus], 0, 0);
                }
            }
        }
//        else
//        {
//            if(Player[nPlay.MySlot + 1].Character == 3 || Player[nPlay.MySlot + 1].Character == 4)
//            {
////                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
////                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
////                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX::HeartMask(1).hdc, 0, 0, vbSrcAnd;
//                if(Player[nPlay.MySlot + 1].Hearts > 0)
//                {
//                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
//                }
//                else
//                {
//                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C - 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
//                }
//                if(Player[nPlay.MySlot + 1].Hearts > 1)
//                {
//                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
//                }
//                else
//                {
//                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
//                }
//                if(Player[nPlay.MySlot + 1].Hearts > 2)
//                {
//                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[1], 0, 0);
//                }
//                else
//                {
//                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Heart[1].w / 2 + C + 32, 16, GFX.Heart[1].w, GFX.Heart[1].h, GFX.Heart[2], 0, 0);
//                }
//            }
//            else
//            {
//                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2, 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX::ContainerMask(0).hdc, 0, 0, vbSrcAnd;
//                BitBlt myBackBuffer, vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2, 16, GFX.Container[1].w + B, GFX.Container[1].h, GFX::Container(0).hdc, 0, 0, vbSrcPaint;
//                if(Player[nPlay.MySlot + 1].HeldBonus > 0)
//                {
//                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12, 16 + 12, NPCWidth[Player[nPlay.MySlot + 1].HeldBonus], NPCHeight[Player[nPlay.MySlot + 1].HeldBonus], GFXNPCMask[Player[nPlay.MySlot + 1].HeldBonus], 0, 0);
//                    frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12, 16 + 12, NPCWidth[Player[nPlay.MySlot + 1].HeldBonus], NPCHeight[Player[nPlay.MySlot + 1].HeldBonus], GFXNPC[Player[nPlay.MySlot + 1].HeldBonus], 0, 0);
//                }
//            }
//        }

        if(Player[1].Character == 5 && Player[1].Bombs > 0)
        {

            frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 34 + C, 52, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[8], 0, 0);
            frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 - 10 + C, 53, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
            SuperPrint(std::to_string(Player[1].Bombs), 1, 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 12 + C, 53);
        }

        // Print coins on the screen
        if(Player[1].HasKey == true)
        {
            frmMain.renderTexture(-24 + 20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[0], 0, 0);
        }
        if(Player[1].Character == 5)
        {
            frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[6], 0, 0);
        }
        else
        {
            frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96, 16 + 10, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[2], 0, 0);
        }
        frmMain.renderTexture(20 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 96 + 8 + GFX.Interface[2].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
        SuperPrint(std::to_string(Coins), 1, 20 - (std::to_string(Coins).size() - 1) * 18 + vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface[3].w, 16 + 11);
        // Print Score
        SuperPrint(std::to_string(Score), 1,
                   20 - (std::to_string(Score).size() - 1) * 18 +
                   float(vScreen[Z].Width) / 2.0f - GFX.Container[1].w / 2 + 80 + 12 + 4 + 18 + 32 + GFX.Interface[3].w,
                   16 + 31);
        // Print lives on the screen
        frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 - 16, 16 + 10, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
        frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 11, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
        SuperPrint(std::to_string(int(Lives)), 1, vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w, 16 + 11);
        // Print stars on the screen
        if(numStars > 0)
        {
            frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122, 16 + 30, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
            frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 10 + GFX.Interface[1].w, 16 + 31, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
            SuperPrint(std::to_string(numStars), 1, vScreen[Z].Width / 2.0 - GFX.Container[1].w / 2 + C - 122 + 12 + 18 + GFX.Interface[5].w, 16 + 31);
        }
    }
    if(BattleIntro > 0)
    {
        if(BattleIntro > 45 || BattleIntro % 2 == 1)
        {
            frmMain.renderTexture(vScreen[Z].Width / 2.0 - GFX.BMVs.w / 2, -96 + vScreen[Z].Height / 2.0 - GFX.BMVs.h / 2, GFX.BMVs.w, GFX.BMVs.h, GFX.BMVs, 0, 0);
            frmMain.renderTexture(-50 + vScreen[Z].Width / 2.0 - GFX.CharacterName[Player[1].Character].w, -96 + vScreen[Z].Height / 2.0 - GFX.CharacterName[Player[1].Character].h / 2, GFX.CharacterName[Player[1].Character].w, GFX.CharacterName[Player[1].Character].h, GFX.CharacterName[Player[1].Character], 0, 0);
            frmMain.renderTexture(50 + vScreen[Z].Width / 2.0, -96 + vScreen[Z].Height / 2.0 - GFX.CharacterName[Player[2].Character].h / 2, GFX.CharacterName[Player[2].Character].w, GFX.CharacterName[Player[2].Character].h, GFX.CharacterName[Player[2].Character], 0, 0);
        }
    }
    if(BattleOutro > 0)
    {
        frmMain.renderTexture(10 + vScreen[Z].Width / 2.0, -96 + vScreen[Z].Height / 2.0 - GFX.BMWin.h / 2, GFX.BMWin.w, GFX.BMWin.h, GFX.BMWin, 0, 0);
        frmMain.renderTexture(-10 + vScreen[Z].Width / 2.0 - GFX.CharacterName[Player[BattleWinner].Character].w, -96 + vScreen[Z].Height / 2.0 - GFX.CharacterName[Player[BattleWinner].Character].h / 2, GFX.CharacterName[Player[BattleWinner].Character].w, GFX.CharacterName[Player[BattleWinner].Character].h, GFX.CharacterName[Player[BattleWinner].Character], 0, 0);
    }
}

int pfrX(int plrFrame)
{
    int A;
    A = plrFrame;
    A = A - 50;
    while(A > 100)
        A = A - 100;
    if(A > 90)
        A = 9;
    else if(A > 90)
        A = 9;
    else if(A > 80)
        A = 8;
    else if(A > 70)
        A = 7;
    else if(A > 60)
        A = 6;
    else if(A > 50)
        A = 5;
    else if(A > 40)
        A = 4;
    else if(A > 30)
        A = 3;
    else if(A > 20)
        A = 2;
    else if(A > 10)
        A = 1;
    else
        A = 0;
    return A * 100;
}

int pfrY(int plrFrame)
{
    int A;
    A = plrFrame;
    A = A - 50;
    while(A > 100)
        A = A - 100;
    A = A - 1;
    while(A > 9)
        A = A - 10;
    return A * 100;
}

void GameThing()
{
    int A = 0;
//    int B = 0;
    RangeArr<Player_t, 1, 2> tempPlayer;
    tempPlayer[1] = Player[1];
    tempPlayer[2] = Player[2];

    SetupPlayers();
    Player[1].Location.SpeedY = 0;
    Player[2].Location.SpeedY = 0;

    if(numPlayers == 1)
    {
        Player[1].Location.X = ScreenW / 2.0 - Player[1].Location.Width / 2.0;
        Player[1].Location.Y = ScreenH / 2.0 - Player[1].Location.Height + 24;
        Player[1].Direction = 1;
    }
    else
    {
        Player[1].Location.X = ScreenW / 2.0 - Player[1].Location.Width / 2.0 - 30;
        Player[1].Location.Y = ScreenH / 2.0 - Player[1].Location.Height + 24;
        Player[1].Direction = -1;
        Player[2].Location.X = ScreenW / 2.0 - Player[2].Location.Width / 2.0 + 32;
        Player[2].Location.Y = ScreenH / 2.0 - Player[2].Location.Height + 24;
        Player[2].Direction = 1;
    }

    PlayerFrame(1);
    PlayerFrame(2);

    frmMain.clearBuffer();

    for(A = 1; A <= numPlayers; A++)
    {
        DrawPlayer(A, 0);
    }

    frmMain.renderTexture(ScreenW / 2.0 - 46, ScreenH / 2.0 + 31, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
    frmMain.renderTexture(ScreenW / 2.0 - GFX.Interface[1].w / 2, ScreenH / 2.0 + 32, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
    SuperPrint(std::to_string(int(Lives)), 1, ScreenW / 2.0 + 12, ScreenH / 2.0 + 32);

    frmMain.repaint();

    Player[1] = tempPlayer[1];
    Player[2] = tempPlayer[2];
    DoEvents();
}

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

                    frmMain.renderTexture(vScreenX[Z] + Player[A].YoshiTongueX - C - 1, vScreenY[Z] + Player[A].YoshiTongue.Y, Player[A].YoshiTongueLength + 2, 16, GFX.Tongue[2], 0, 0, s, s, s);

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
                    DrawFrozenNPC( Z, Player[A].HoldingNPC);
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

void ScreenShot()
{
    frmMain.makeShot();
    PlaySound(12);
    TakeScreen = false;
}

void DrawFrozenNPC(int Z, int A)
{
    auto &n = NPC[A];
    if((vScreenCollision(Z, n.Location) Or
        vScreenCollision(Z, newLoc(n.Location.X - (NPCWidthGFX[n.Type] - n.Location.Width) / 2,
                            n.Location.Y, CDbl(NPCWidthGFX[n.Type]), CDbl(NPCHeight[n.Type])))) And !n.Hidden)
    {
// draw npc
        float c = n.Shadow ? 0.f : 1.f;

        frmMain.renderTexture(int(vScreenX[Z] + n.Location.X + 2),
                              int(vScreenY[Z] + n.Location.Y + 2),
                              int(n.Location.Width - 4),
                              int(n.Location.Height - 4),
                              GFXNPCBMP[int(n.Special)],
                              2, 2 + int(n.Special2) * NPCHeight[int(n.Special)], c, c, c);

        // draw ice
        frmMain.renderTexture(int(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type]),
                              int(vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type]),
                              int(n.Location.Width - 6), int(n.Location.Height - 6),
                              GFXNPCBMP[n.Type], 0, 0, c, c, c);
        frmMain.renderTexture(int(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] + n.Location.Width - 6),
                              int(vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type]),
                              6, int(n.Location.Height - 6),
                              GFXNPCBMP[n.Type], 128 - 6, 0, c, c, c);
        frmMain.renderTexture(int(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type]),
                              int(vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] + n.Location.Height - 6),
                              int(n.Location.Width - 6), 6,
                              GFXNPCBMP[n.Type], 0, 128 - 6, c, c, c);
        frmMain.renderTexture(int(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] + n.Location.Width - 6),
                              int(vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] + n.Location.Height - 6),
                              6, 6, GFXNPCBMP[n.Type],
                              128 - 6, 128 - 6, c, c, c);
    }
}
