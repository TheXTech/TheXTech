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
//        BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness;
//    }
//    else
//    {
//        BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness;
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
//                BitBlt myBackBuffer, vScreenX[Z] + Tile[A].Location.X, vScreenY[Z] + Tile[A].Location.Y, Tile[A].Location.Width, Tile[A].Location.Height, GFXTile[Tile[A].Type], 0, TileHeight[Tile[A].Type] * TileFrame[Tile[A].Type], vbSrcCopy;
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
//                BitBlt myBackBuffer, vScreenX[Z] + Scene[A].Location.X, vScreenY[Z] + Scene[A].Location.Y, Scene[A].Location.Width, Scene[A].Location.Height, GFXSceneMask[Scene[A].Type], 0, SceneHeight[Scene[A].Type] * SceneFrame[Scene[A].Type], vbSrcAnd;
//                BitBlt myBackBuffer, vScreenX[Z] + Scene[A].Location.X, vScreenY[Z] + Scene[A].Location.Y, Scene[A].Location.Width, Scene[A].Location.Height, GFXScene[Scene[A].Type], 0, SceneHeight[Scene[A].Type] * SceneFrame[Scene[A].Type], vbSrcPaint;
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
//                BitBlt myBackBuffer, vScreenX[Z] + WorldPath[A].Location.X, vScreenY[Z] + WorldPath[A].Location.Y, WorldPath[A].Location.Width, WorldPath[A].Location.Height, GFXPathMask[WorldPath[A].Type], 0, 0, vbSrcAnd;
//                BitBlt myBackBuffer, vScreenX[Z] + WorldPath[A].Location.X, vScreenY[Z] + WorldPath[A].Location.Y, WorldPath[A].Location.Width, WorldPath[A].Location.Height, GFXPath[WorldPath[A].Type], 0, 0, vbSrcPaint;
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
//                    BitBlt myBackBuffer, vScreenX[Z] + WorldLevel[A].Location.X, vScreenY[Z] + WorldLevel[A].Location.Y, WorldLevel[A].Location.Width, WorldLevel[A].Location.Height, GFXLevelMask[0], 0, 0, vbSrcAnd;
//                    BitBlt myBackBuffer, vScreenX[Z] + WorldLevel[A].Location.X, vScreenY[Z] + WorldLevel[A].Location.Y, WorldLevel[A].Location.Width, WorldLevel[A].Location.Height, GFXLevel[0], 0, 0, vbSrcPaint;
                    frmMain.renderTexture(vScreenX[Z] + WorldLevel[A].Location.X,
                                          vScreenY[Z] + WorldLevel[A].Location.Y,
                                          WorldLevel[A].Location.Width,
                                          WorldLevel[A].Location.Height,
                                          GFXLevelBMP[0], 0, 0);
                }
                if(WorldLevel[A].Path2 == true)
                {
//                    BitBlt myBackBuffer, vScreenX[Z] + WorldLevel[A].Location.X - 16, vScreenY[Z] + 8 + WorldLevel[A].Location.Y, 64, 32, GFXLevelMask[29], 0, 0, vbSrcAnd;
//                    BitBlt myBackBuffer, vScreenX[Z] + WorldLevel[A].Location.X - 16, vScreenY[Z] + 8 + WorldLevel[A].Location.Y, 64, 32, GFXLevel[29], 0, 0, vbSrcPaint;
                    frmMain.renderTexture(vScreenX[Z] + WorldLevel[A].Location.X - 16,
                                          vScreenY[Z] + 8 + WorldLevel[A].Location.Y,
                                          64, 32,
                                          GFXLevelBMP[29], 0, 0);
                }
                if(GFXLevelBig[WorldLevel[A].Type] == true)
                {
//                    BitBlt myBackBuffer, vScreenX[Z] + WorldLevel[A].Location.X - (GFXLevelWidth[WorldLevel[A].Type] - 32) / 2.0, vScreenY[Z] + WorldLevel[A].Location.Y - GFXLevelHeight[WorldLevel[A].Type] + 32, GFXLevelWidth[WorldLevel[A].Type], GFXLevelHeight[WorldLevel[A].Type], GFXLevelMask[WorldLevel[A].Type], 0, 32 * LevelFrame[WorldLevel[A].Type], vbSrcAnd;
//                    BitBlt myBackBuffer, vScreenX[Z] + WorldLevel[A].Location.X - (GFXLevelWidth[WorldLevel[A].Type] - 32) / 2.0, vScreenY[Z] + WorldLevel[A].Location.Y - GFXLevelHeight[WorldLevel[A].Type] + 32, GFXLevelWidth[WorldLevel[A].Type], GFXLevelHeight[WorldLevel[A].Type], GFXLevel[WorldLevel[A].Type], 0, 32 * LevelFrame[WorldLevel[A].Type], vbSrcPaint;
                    frmMain.renderTexture(vScreenX[Z] + WorldLevel[A].Location.X - (GFXLevelWidth[WorldLevel[A].Type] - 32) / 2.0,
                                          vScreenY[Z] + WorldLevel[A].Location.Y - GFXLevelHeight[WorldLevel[A].Type] + 32,
                                          GFXLevelWidth[WorldLevel[A].Type], GFXLevelHeight[WorldLevel[A].Type],
                                          GFXLevelBMP[WorldLevel[A].Type], 0, 32 * LevelFrame[WorldLevel[A].Type]);
                }
                else
                {
//                    BitBlt myBackBuffer, vScreenX[Z] + WorldLevel[A].Location.X, vScreenY[Z] + WorldLevel[A].Location.Y, WorldLevel[A].Location.Width, WorldLevel[A].Location.Height, GFXLevelMask[WorldLevel[A].Type], 0, 32 * LevelFrame[WorldLevel[A].Type], vbSrcAnd;
//                    BitBlt myBackBuffer, vScreenX[Z] + WorldLevel[A].Location.X, vScreenY[Z] + WorldLevel[A].Location.Y, WorldLevel[A].Location.Width, WorldLevel[A].Location.Height, GFXLevel[WorldLevel[A].Type], 0, 32 * LevelFrame[WorldLevel[A].Type], vbSrcPaint;
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
//        BitBlt myBackBuffer, vScreenX[Z] + WorldPlayer[1].Location.X, vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayerMask[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame, vbSrcAnd;
//        BitBlt myBackBuffer, vScreenX[Z] + WorldPlayer[1].Location.X, vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayer[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame, vbSrcPaint;
        frmMain.renderTexture(vScreenX[Z] + WorldPlayer[1].Location.X,
                              vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight,
                              WorldPlayer[1].Location.Width, WPHeight,
                              GFXPlayerBMP[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);

//        BitBlt myBackBuffer, 0, 0, 800, 130, GFX::Interface(4).hdc, 0, 0, vbSrcCopy;
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

        SuperPrint(std::to_string(Lives), 1, 32 + (48 * A) + 62, 112);
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
            SuperPrint(std::to_string(PrintFPS), 1, 8, 8);
        }
//        BitBlt frmMain.hdc, 0, 0, ScreenW, ScreenH, myBackBuffer, 0, 0, vbSrcCopy
//        StretchBlt frmMain::hdc, 0, 0, frmMain::ScaleWidth, frmMain::ScaleHeight, myBackBuffer, 0, 0, 800, 600, vbSrcCopy;
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
                if(BackgroundHasNoMask[Background[A].Type] == false)
                {
                    if(vScreenCollision(Z, Background[A].Location) && Background[A].Hidden == false)
                    {
                        frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X, vScreenY[Z] + Background[A].Location.Y, BackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackgroundBMP[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                    }
                }
                else
                {
                    if(vScreenCollision(Z, Background[A].Location) && Background[A].Hidden == false)
                    {
                        frmMain.renderTexture(vScreenX[Z] + Background[A].Location.X, vScreenY[Z] + Background[A].Location.Y, GFXBackgroundWidth[Background[A].Type], BackgroundHeight[Background[A].Type], GFXBackgroundBMP[Background[A].Type], 0, BackgroundHeight[Background[A].Type] * BackgroundFrame[Background[A].Type]);
                    }
                }
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
            if((NPC[A].Effect == 208 || NPCIsAVine[NPC[A].Type] == true ||
                NPC[A].Type == 209 || NPC[A].Type == 159 || NPC[A].Type == 245 ||
                NPC[A].Type == 8 || NPC[A].Type == 93 || NPC[A].Type == 74 ||
                NPC[A].Type == 256 || NPC[A].Type == 257 || NPC[A].Type == 51 ||
                NPC[A].Type == 52 || NPC[A].Effect == 1 || NPC[A].Effect == 3 ||
                NPC[A].Effect == 4 || (NPC[A].Type == 45 && NPC[A].Special == 0.0)) &&
                    NPC[A].standingOnPlayer == 0 && (NPC[A].Generator == false || LevelEditor == true) ||
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
            if(Player[A].Dead == false && Player[A].Immune2 == false && Player[A].TimeToLive == 0 && !(Player[A].Effect == 3 || Player[A].Effect == 5) && Player[A].Mount == 2)
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
            if((((NPC[A].HoldingPlayer > 0 && Player[NPC[A].HoldingPlayer].Effect != 3) ||
               (NPC[A].Type == 50 && NPC[A].standingOnPlayer == 0) ||
                NPC[A].Type == 17 && NPC[A].CantHurt > 0) ||
                NPC[A].Effect == 5) && !(NPC[A].Type == 91) && Player[NPC[A].HoldingPlayer].Dead == false)
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
                    if(MessageText == "")
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
                        frmMain.renderTexture(400 - GFX.TextBox.w / 2 + X, BoxY + Y + Y, GFX.TextBox.h, 20, GFX.TextBox, 0, 0);
                        BoxY = BoxY + 10;
                        tempBool = false;
                        do
                        {
                            B = 0;
                            int tempVar6 = int(SuperText.size());
                            for(A = 1; A <= tempVar6; A++)
                            {
                                if(SuperText[A - 1] == ' ' || A == SuperText.size())
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
                SuperPrint(fmt::format_ne("%d", PrintFPS), 1, 8, 8);
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
            {
                if(ScreenShake > 0)
                {
                    ScreenShake--;
                    A = (std::rand() % ScreenShake * 4) - ScreenShake * 2;
                    B = (std::rand() % ScreenShake * 4) - ScreenShake * 2;
                    frmMain.offsetViewport(A, B);
                    if(ScreenShake == 0)
                        frmMain.offsetViewport(0, 0);
                }
            }
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
//    If ScreenType = 8 Then
//        If Player(nPlay.MySlot + 1).Mount = 2 Then Player(nPlay.MySlot + 1).Location.Height = 0
//        vScreenX(1) = -Player(nPlay.MySlot + 1).Location.X + (vScreen(1).Width * 0.5) - Player(nPlay.MySlot + 1).Location.Width / 2
//        vScreenY(1) = -Player(nPlay.MySlot + 1).Location.Y + (vScreen(1).Height * 0.5) - vScreenYOffset - Player(nPlay.MySlot + 1).Location.Height
//        vScreenX(1) = vScreenX(1) - vScreen(1).tempX
//        vScreenY(1) = vScreenY(1) - vScreen(1).TempY
//        If -vScreenX(1) < level(Player(nPlay.MySlot + 1).Section).X Then vScreenX(1) = -level(Player(nPlay.MySlot + 1).Section).X
//        If -vScreenX(1) + vScreen(1).Width > level(Player(nPlay.MySlot + 1).Section).Width Then vScreenX(1) = -(level(Player(nPlay.MySlot + 1).Section).Width - vScreen(1).Width)
//        If -vScreenY(1) < level(Player(nPlay.MySlot + 1).Section).Y Then vScreenY(1) = -level(Player(nPlay.MySlot + 1).Section).Y
//        If -vScreenY(1) + vScreen(1).Height > level(Player(nPlay.MySlot + 1).Section).Height Then vScreenY(1) = -(level(Player(nPlay.MySlot + 1).Section).Height - vScreen(1).Height)
//        If vScreen(1).TempDelay > 0 Then
//            vScreen(1).TempDelay = vScreen(1).TempDelay - 1
//        Else
//            If vScreen(1).tempX > 0 Then vScreen(1).tempX = vScreen(1).tempX - 1
//            If vScreen(1).tempX < 0 Then vScreen(1).tempX = vScreen(1).tempX + 1
//            If vScreen(1).TempY > 0 Then vScreen(1).TempY = vScreen(1).TempY - 1
//            If vScreen(1).TempY < 0 Then vScreen(1).TempY = vScreen(1).TempY + 1
//        End If
//        If Player(nPlay.MySlot + 1).Mount = 2 Then Player(nPlay.MySlot + 1).Location.Height = 128
//    Else
//        If Player(A).Mount = 2 Then Player(A).Location.Height = 0
//        vScreenX(A) = -Player(A).Location.X + (vScreen(A).Width * 0.5) - Player(A).Location.Width / 2
//        vScreenY(A) = -Player(A).Location.Y + (vScreen(A).Height * 0.5) - vScreenYOffset - Player(A).Location.Height
//        vScreenX(A) = vScreenX(A) - vScreen(A).tempX
//        vScreenY(A) = vScreenY(A) - vScreen(A).TempY
//        If -vScreenX(A) < level(Player(A).Section).X Then vScreenX(A) = -level(Player(A).Section).X
//        If -vScreenX(A) + vScreen(A).Width > level(Player(A).Section).Width Then vScreenX(A) = -(level(Player(A).Section).Width - vScreen(A).Width)
//        If -vScreenY(A) < level(Player(A).Section).Y Then vScreenY(A) = -level(Player(A).Section).Y
//        If -vScreenY(A) + vScreen(A).Height > level(Player(A).Section).Height Then vScreenY(A) = -(level(Player(A).Section).Height - vScreen(A).Height)
//        If vScreen(A).TempDelay > 0 Then
//            vScreen(A).TempDelay = vScreen(A).TempDelay - 1
//        Else
//            If vScreen(A).tempX > 0 Then vScreen(A).tempX = vScreen(A).tempX - 1
//            If vScreen(A).tempX < 0 Then vScreen(A).tempX = vScreen(A).tempX + 1
//            If vScreen(A).TempY > 0 Then vScreen(A).TempY = vScreen(A).TempY - 1
//            If vScreen(A).TempY < 0 Then vScreen(A).TempY = vScreen(A).TempY + 1
//        End If
//        If Player(A).Mount = 2 Then Player(A).Location.Height = 128
//    End If
}

// Get the average screen position for all players
void GetvScreenAverage()
{
//    Dim A As Integer
//    Dim B As Integer
//    Dim OldX As Double
//    Dim OldY As Double
//    OldX = vScreenX(1)
//    OldY = vScreenY(1)
//    vScreenX(1) = 0
//    vScreenY(1) = 0
//    For A = 1 To numPlayers
//        If Player(A).Dead = False And Player(A).Effect <> 6 Then
//            vScreenX(1) = vScreenX(1) - Player(A).Location.X - Player(A).Location.Width / 2
//            If Player(A).Mount = 2 Then
//                vScreenY(1) = vScreenY(1) - Player(A).Location.Y
//            Else
//                vScreenY(1) = vScreenY(1) - Player(A).Location.Y - Player(A).Location.Height
//            End If
//            B = B + 1
//        End If
//    Next A
//    A = 1
//    If B = 0 Then
//        If GameMenu = True Then
//            vScreenX(1) = -level(0).X
//            B = 1
//        Else
//            Exit Sub
//        End If
//    End If
//    vScreenX(1) = (vScreenX(1) / B) + (ScreenW * 0.5)
//    vScreenY(1) = (vScreenY(1) / B) + (ScreenH * 0.5) - vScreenYOffset
//    If -vScreenX(A) < level(Player(1).Section).X Then vScreenX(A) = -level(Player(1).Section).X
//    If -vScreenX(A) + ScreenW > level(Player(1).Section).Width Then vScreenX(A) = -(level(Player(1).Section).Width - ScreenW)
//    If -vScreenY(A) < level(Player(1).Section).Y Then vScreenY(A) = -level(Player(1).Section).Y
//    If -vScreenY(A) + ScreenH > level(Player(1).Section).Height Then vScreenY(A) = -(level(Player(1).Section).Height - ScreenH)
//    If GameMenu = True Then
//        If vScreenX(1) > OldX Then
//            If vScreenX(1) = -level(0).X Then
//                vScreenX(1) = OldX + 20
//            Else
//                vScreenX(1) = OldX
//            End If
//        ElseIf vScreenX(1) < OldX - 10 Then
//            vScreenX(1) = OldX - 10
//        End If
//    End If
}

// Get the average screen position for all players with no level edge detection
void GetvScreenAverage2()
{
//    Dim A As Integer
//    Dim B As Integer
//    vScreenX(1) = 0
//    vScreenY(1) = 0
//    For A = 1 To numPlayers
//        If Player(A).Dead = False Then
//            vScreenX(1) = vScreenX(1) - Player(A).Location.X - Player(A).Location.Width / 2
//            If Player(A).Mount = 2 Then
//                vScreenY(1) = vScreenY(1) - Player(A).Location.Y
//            Else
//                vScreenY(1) = vScreenY(1) - Player(A).Location.Y - Player(A).Location.Height
//            End If
//            B = B + 1
//        End If
//    Next A
//    A = 1
//    If B = 0 Then Exit Sub
//    vScreenX(1) = (vScreenX(1) / B) + (ScreenW * 0.5)
//    vScreenY(1) = (vScreenY(1) / B) + (ScreenH * 0.5) - vScreenYOffset
}

void SetupGraphics()
{
    //DUMMY AND USELESS

//    'Creates the back buffer for the main game
//    'myBackBuffer = CreateCompatibleDC(GetDC(0))
//    'myBufferBMP = CreateCompatibleBitmap(GetDC(0), screenw, screenh)

//    myBackBuffer = CreateCompatibleDC(frmMain.hdc)
//    myBufferBMP = CreateCompatibleBitmap(frmMain.hdc, ScreenW, ScreenH)
//    SelectObject myBackBuffer, myBufferBMP
//    GFX.Split(2).Width = ScreenW
//    GFX.Split(2).Height = ScreenH
//    'GFX.BackgroundColor(1).Width = Screen.Width
//    'GFX.BackgroundColor(1).Height = Screen.Height
//    'GFX.BackgroundColor(2).Width = Screen.Width
//    'GFX.BackgroundColor(2).Height = Screen.Height
}

void SetupEditorGraphics()
{
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
//    If ScreenType = 0 Then 'Follows Player 1
    case 0:
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = False;
        break;
//    ElseIf ScreenType = 1 Then 'Split Screen vertical
    case 1:
        vScreen[1].Height = ScreenH / 2;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Height = ScreenH / 2;
        vScreen[2].Width = ScreenW;
        vScreen[2].Left = 0;
        vScreen[2].Top = ScreenH / 2;
        break;
//    ElseIf ScreenType = 2 Then 'Follows all players
    case 2:
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = False;
        break;
//    ElseIf ScreenType = 3 Then 'Follows all players. Noone leaves the screen
    case 3:
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = False;
        break;
//    ElseIf ScreenType = 4 Then 'Split Screen horizontal
    case 4:
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW / 2;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Height = ScreenH;
        vScreen[2].Width = ScreenW / 2;
        vScreen[2].Left = ScreenW / 2;
        vScreen[2].Top = 0;
        break;
//    ElseIf ScreenType = 5 Then 'Dynamic screen detection
    case 5:
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Visible = False;
        break;
//    ElseIf ScreenType = 6 Then 'VScreen Coop
    case 6:
        vScreen[1].Height = ScreenH;
        vScreen[1].Width = ScreenW;
        vScreen[1].Left = 0;
        vScreen[1].Top = 0;
        vScreen[2].Height = ScreenH;
        vScreen[2].Width = ScreenW;
        vScreen[2].Left = 0;
        vScreen[2].Top = 0;
        break;
//    ElseIf ScreenType = 7 Then
    case 7:
        vScreen[1].Left = 0;
        vScreen[1].Width = 800;
        vScreen[1].Top = 0;
        vScreen[1].Height = 600;
        vScreen[2].Visible = False;
        break;
//    ElseIf ScreenType = 8 Then 'netplay
    case 8:
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
//    Dim A As Integer
//    GetvScreenAverage
//    For A = 1 To numPlayers
//        If Player(A).Effect = 6 Then Exit Sub
//    Next A
//    For A = 1 To numPlayers
//        If Player(A).Mount = 2 Then Player(A).Location.Height = 0
//    Next A
//    If CheckDead = 0 Then
//        If Player(1).Section = Player(2).Section Then
//            If level(Player(1).Section).Width - level(Player(1).Section).X > ScreenW And (((vScreen(2).Visible = False And Player(2).Location.X + vScreenX(1) >= ScreenW * 0.75 - Player(2).Location.Width / 2) Or (vScreen(2).Visible = True And Player(2).Location.X + vScreenX(1) >= ScreenW * 0.75 - Player(2).Location.Width / 2)) And (Player(1).Location.X < level(Player(1).Section).Width - ScreenW * 0.75 - Player(1).Location.Width / 2)) Then
//                vScreen(2).Height = ScreenH
//                vScreen(2).Width = ScreenW / 2
//                vScreen(2).Left = ScreenW / 2
//                vScreen(2).Top = 0
//                vScreen(1).Height = ScreenH
//                vScreen(1).Width = ScreenW / 2
//                vScreen(1).Left = 0
//                vScreen(1).Top = 0
//                GetvScreenAverage2
//                If DScreenType <> 1 Then PlaySound 13
//                For A = 1 To 2
//                    vScreen(A).TempDelay = 200
//                    vScreen(A).tempX = 0
//                    vScreen(A).TempY = -vScreenY(1) + ScreenH * 0.5 - Player(A).Location.Y - vScreenYOffset - Player(A).Location.Height
//                Next A
//                vScreen(2).Visible = True
//                DScreenType = 1
//            ElseIf level(Player(1).Section).Width - level(Player(1).Section).X > ScreenW And (((vScreen(2).Visible = False And Player(1).Location.X + vScreenX(1) >= ScreenW * 0.75 - Player(1).Location.Width / 2) Or (vScreen(2).Visible = True And Player(1).Location.X + vScreenX(2) >= ScreenW * 0.75 - Player(1).Location.Width / 2)) And (Player(2).Location.X < level(Player(1).Section).Width - ScreenW * 0.75 - Player(2).Location.Width / 2)) Then
//                vScreen(1).Height = ScreenH
//                vScreen(1).Width = ScreenW / 2
//                vScreen(1).Left = ScreenW / 2
//                vScreen(1).Top = 0
//                vScreen(2).Height = ScreenH
//                vScreen(2).Width = ScreenW / 2
//                vScreen(2).Left = 0
//                vScreen(2).Top = 0
//                GetvScreenAverage2
//                If DScreenType <> 2 Then PlaySound 13
//                For A = 1 To 2
//                    vScreen(A).TempDelay = 200
//                    vScreen(A).tempX = 0
//                    vScreen(A).TempY = -vScreenY(1) + ScreenH * 0.5 - Player(A).Location.Y - vScreenYOffset - Player(A).Location.Height
//                Next A
//                DScreenType = 2
//                vScreen(2).Visible = True
//            ElseIf (level(Player(1).Section).Height - level(Player(1).Section).Y > ScreenH And ((vScreen(2).Visible = False And Player(1).Location.Y + vScreenY(1) >= ScreenH * 0.75 - vScreenYOffset - Player(1).Location.Height) Or (vScreen(2).Visible = True And Player(1).Location.Y + vScreenY(2) >= ScreenH * 0.75 - vScreenYOffset - Player(1).Location.Height)) And (Player(2).Location.Y < level(Player(1).Section).Height - ScreenH * 0.75 - vScreenYOffset - Player(2).Location.Height)) Then
//                vScreen(1).Height = ScreenH / 2
//                vScreen(1).Width = ScreenW
//                vScreen(1).Left = 0
//                vScreen(1).Top = ScreenH / 2
//                vScreen(2).Height = ScreenH / 2
//                vScreen(2).Width = ScreenW
//                vScreen(2).Left = 0
//                vScreen(2).Top = 0
//                GetvScreenAverage2
//                If DScreenType <> 3 Then PlaySound 13
//                For A = 1 To 2
//                    vScreen(A).TempDelay = 200
//                    vScreen(A).TempY = 0
//                    vScreen(A).tempX = -vScreenX(1) + ScreenW * 0.5 - Player(A).Location.X - Player(A).Location.Width * 0.5
//                Next A
//                vScreen(2).Visible = True
//                DScreenType = 3
//            ElseIf (level(Player(1).Section).Height - level(Player(1).Section).Y > ScreenH And ((vScreen(2).Visible = False And Player(2).Location.Y + vScreenY(1) >= ScreenH * 0.75 - vScreenYOffset - Player(2).Location.Height) Or (vScreen(2).Visible = True And Player(2).Location.Y + vScreenY(1) >= ScreenH * 0.75 - vScreenYOffset - Player(2).Location.Height)) And (Player(1).Location.Y < level(Player(1).Section).Height - ScreenH * 0.75 - vScreenYOffset - Player(1).Location.Height)) Then
//                vScreen(1).Height = ScreenH / 2
//                vScreen(1).Width = ScreenW
//                vScreen(1).Left = 0
//                vScreen(1).Top = 0
//                vScreen(2).Height = ScreenH / 2
//                vScreen(2).Width = ScreenW
//                vScreen(2).Left = 0
//                vScreen(2).Top = ScreenH / 2
//                GetvScreenAverage2
//                If DScreenType <> 4 Then PlaySound 13
//                For A = 1 To 2
//                    vScreen(A).TempDelay = 200
//                    vScreen(A).TempY = 0
//                    vScreen(A).tempX = -vScreenX(1) + ScreenW * 0.5 - Player(A).Location.X - Player(A).Location.Width * 0.5
//                Next A
//                vScreen(2).Visible = True
//                DScreenType = 4
//            Else
//                If vScreen(2).Visible = True Then
//                    If DScreenType <> 5 Then PlaySound 13
//                    vScreen(2).Visible = False
//                    vScreen(1).Height = ScreenH
//                    vScreen(1).Width = ScreenW
//                    vScreen(1).Left = 0
//                    vScreen(1).Top = 0
//                    vScreen(1).tempX = 0
//                    vScreen(1).TempY = 0
//                    vScreen(2).tempX = 0
//                    vScreen(2).TempY = 0
//                End If
//                DScreenType = 5
//            End If
//            For A = 1 To 2
//                If vScreen(A).TempY > (vScreen(A).Height * 0.25) Then vScreen(A).TempY = (vScreen(A).Height * 0.25)
//                If vScreen(A).TempY < -(vScreen(A).Height * 0.25) Then vScreen(A).TempY = -(vScreen(A).Height * 0.25)
//                If vScreen(A).tempX > (vScreen(A).Width * 0.25) Then vScreen(A).tempX = (vScreen(A).Width * 0.25)
//                If vScreen(A).tempX < -(vScreen(A).Width * 0.25) Then vScreen(A).tempX = -(vScreen(A).Width * 0.25)
//            Next A
//        Else
//            vScreen(1).Height = ScreenH / 2
//            vScreen(1).Width = ScreenW
//            vScreen(1).Left = 0
//            vScreen(1).Top = 0
//            vScreen(2).Height = ScreenH / 2
//            vScreen(2).Width = ScreenW
//            vScreen(2).Left = 0
//            vScreen(2).Top = ScreenH / 2
//            vScreen(1).tempX = 0
//            vScreen(1).TempY = 0
//            vScreen(2).tempX = 0
//            vScreen(2).TempY = 0
//            GetvScreenAverage2
//            If DScreenType <> 6 Then PlaySound 13
//            DScreenType = 6
//            vScreen(2).Visible = True
//        End If
//    Else
//        If vScreen(2).Visible = True Then
//            vScreen(2).Visible = False
//            vScreen(1).Visible = False
//            vScreen(1).Height = ScreenH
//            vScreen(1).Width = ScreenW
//            vScreen(1).Left = 0
//            vScreen(1).Top = 0
//            vScreen(1).Visible = True
//            vScreen(1).tempX = 0
//            vScreen(1).TempY = 0
//            vScreen(2).tempX = 0
//            vScreen(2).TempY = 0
//        End If
//    End If
//    For A = 1 To numPlayers
//        If Player(A).Mount = 2 Then Player(A).Location.Height = 128
//    Next A
}

void SuperPrint(std::string SuperWords, int Font, float X, float Y)
{
//    On Error Resume Next
//    Dim A As Integer
    int A = 0;
//    Dim B As Integer
    int B = 0;
//    Dim C As Integer
    int C = 0;
//    Dim Words As String
//    Words = SuperWords
//    If Font = 1 Then
    if(Font == 1) {
//        Do While Len(Words) > 0
        for(auto c : SuperWords)
        {
//            If Left(Words, 1) <> " " Then
            if(c != ' ' && c >= 0 && c <= 9)
            {
//                BitBlt myBackBuffer, X + B, Y, 16, 14, GFX.Font1M(Left(Words, 1)).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, X + B, Y, 16, 14, GFX.Font1(Left(Words, 1)).hdc, 0, 0, vbSrcPaint
                frmMain.renderTexture(int(X + B), int(Y), 16, 14, GFX.Font1[c], 0, 0);
//                B = B + 18
                B += 18;
//            End If
            }
//            Words = Right(Words, Len(Words) - 1)
//        Loop
        }
//    ElseIf Font = 2 Then
    } else if(Font == 2) {
//        Do While Len(Words) > 0
        for(auto c : SuperWords)
        {
//            If Asc(Left(Words, 1)) >= 48 And Asc(Left(Words, 1)) <= 57 Then
            if(c >= 48 && c <= 57) {
//                C = (Asc(Left(Words, 1)) - 48) * 16
                C = (c - 48) * 16;
//                BitBlt myBackBuffer, X + B, Y, 15, 17, GFX.Font2(1).hdc, C, 0, vbSrcAnd
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0);
//                B = B + 16
                B += 16;
//            ElseIf Asc(Left(Words, 1)) >= 65 And Asc(Left(Words, 1)) <= 90 Then
            } else if(c >= 65 && c <= 90) {
//                C = (Asc(Left(Words, 1)) - 55) * 16
                C = (c - 55) * 16;
//                BitBlt myBackBuffer, X + B, Y, 15, 17, GFX.Font2(1).hdc, C, 0, vbSrcAnd
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0);
//                B = B + 16
                B += 16;
//            ElseIf Asc(Left(Words, 1)) >= 97 And Asc(Left(Words, 1)) <= 122 Then
            } else if(c >= 97 && c <= 122) {
//                C = (Asc(Left(Words, 1)) - 61) * 16
                C = (c - 61) * 16;
//                BitBlt myBackBuffer, X + B, Y, 15, 17, GFX.Font2(1).hdc, C, 0, vbSrcAnd
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2[1], C, 0);
//                B = B + 16
                B += 16;
//            ElseIf Asc(Left(Words, 1)) >= 33 And Asc(Left(Words, 1)) <= 47 Then
            } else if(c >= 33 && c <= 47) {
//                C = (Asc(Left(Words, 1)) - 33) * 16
                C = (c - 33) * 16;
//                BitBlt myBackBuffer, X + B, Y, 15, 17, GFX.Font2S(1).hdc, C, 0, vbSrcAnd
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0);
//                B = B + 16
                B += 16;
//            ElseIf Asc(Left(Words, 1)) >= 58 And Asc(Left(Words, 1)) <= 64 Then
            } else if(c >= 58 && c <= 64) {
//                C = (Asc(Left(Words, 1)) - 58 + 15) * 16
                C = (c - 58 + 15) * 16;
//                BitBlt myBackBuffer, X + B, Y, 15, 17, GFX.Font2S(1).hdc, C, 0, vbSrcAnd
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0);
//                B = B + 16
                B += 16;
//            ElseIf Asc(Left(Words, 1)) >= 91 And Asc(Left(Words, 1)) <= 96 Then
            } else if(c >= 91 && c <= 96) {
//                C = (Asc(Left(Words, 1)) - 91 + 22) * 16
                C = (c - 91 + 22) * 16;
//                BitBlt myBackBuffer, X + B, Y, 15, 17, GFX.Font2S(1).hdc, C, 0, vbSrcAnd
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0);
//                B = B + 16
                B += 16;
//            ElseIf Asc(Left(Words, 1)) >= 123 And Asc(Left(Words, 1)) <= 125 Then
            } else if(c >= 123 && c <= 125) {
//                C = (Asc(Left(Words, 1)) - 123 + 28) * 16
                C = (c - 123 + 28) * 16;
//                BitBlt myBackBuffer, X + B, Y, 15, 17, GFX.Font2S(1).hdc, C, 0, vbSrcAnd
                frmMain.renderTexture(int(X + B), int(Y), 15, 17, GFX.Font2S, C, 0);
//                B = B + 16
                B += 16;
//            Else
            } else {
//                B = B + 16
                B += 16;
//            End If
            }
//            Words = Right(Words, Len(Words) - 1)
//        Loop
        }
//    ElseIf Font = 3 Then
    } else if (Font == 3) {
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
                frmMain.renderTexture(int(X + B), int(Y), 18, 16, GFX.Font2[2], 2, C);
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
    } else if(Font == 4) {
//        Do While Len(Words) > 0
        for(auto c : SuperWords)
        {
//            If Asc(Left(Words, 1)) >= 33 And Asc(Left(Words, 1)) <= 126 Then
            if(c >= 33 && c <= 126)
            {
//                C = (Asc(Left(Words, 1)) - 33) * 16
                C = (c - 33) * 16;
//                BitBlt myBackBuffer, X + B, Y, 18, 16, GFX.Font2(3).hdc, 2, C, vbSrcPaint
                frmMain.renderTexture(int(X + B), int(Y), 18, 16, GFX.Font2[3], 2, C);
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
//    GetCurrentRes
    GetCurrentRes();
//    resChanged = True
    resChanged = true;
//    ChangeRes 800, 600, 16, 60     '800x600 pixels, 16 bit Color, 60Hz
    ChangeRes(800, 600, 16, 60);
//    'SetWindowPos frmMain.hWnd, HWND_TOPMOST, 0, 0, 0, 0, FLAGS
//    Do
//    Loop Until ShowCursor(0) < 0
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

}

void DrawBackground(int S, int Z)
{

}

void PlayerWarpGFX(int A, Location_t tempLocation, float X2, float Y2)
{

}

void NPCWarpGFX(int A, Location_t tempLocation, float X2, float Y2)
{

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
//    Dim A As Integer
//    Dim B As Integer
//    vScreenX(1) = 0
//    vScreenY(1) = 0
//    For A = 1 To numPlayers
//        If Player(A).Dead = False And Player(A).Effect <> 6 Then
//            vScreenX(1) = vScreenX(1) - Player(A).Location.X - Player(A).Location.Width / 2
//            If Player(A).Mount = 2 Then
//                vScreenY(1) = vScreenY(1) - Player(A).Location.Y
//            Else
//                vScreenY(1) = vScreenY(1) - Player(A).Location.Y - Player(A).Location.Height
//            End If
//            B = B + 1
//        End If
//    Next A
//    A = 1
//    If B = 0 Then Exit Sub
//    vScreenX(1) = (vScreenX(1) / B) + (ScreenW * 0.5)
//    vScreenY(1) = (vScreenY(1) / B) + (ScreenH * 0.5) - vScreenYOffset
//    If -vScreenX(A) < level(Player(1).Section).X Then vScreenX(A) = -level(Player(1).Section).X
//    If -vScreenX(A) + ScreenW > level(Player(1).Section).Width Then vScreenX(A) = -(level(Player(1).Section).Width - ScreenW)
//    If -vScreenY(A) < level(Player(1).Section).Y + 100 Then vScreenY(A) = -level(Player(1).Section).Y + 100
//    If -vScreenY(A) + ScreenH > level(Player(1).Section).Height - 100 Then vScreenY(A) = -(level(Player(1).Section).Height - ScreenH) - 100
}

void DoCredits()
{
//    Dim A As Integer
//    If GameMenu = True Then Exit Sub
//    For A = 1 To numCredits
//        With Credit(A)
//            .Location.Y = .Location.Y - 0.8
//            If .Location.Y <= 600 And .Location.Y + .Location.Height >= 0 Then
//                SuperPrint .Text, 4, CSng(.Location.X), CSng(.Location.Y)
//            ElseIf A = numCredits And .Location.Y + .Location.Height < -100 Then
//                CreditChop = CreditChop + 0.4
//                If CreditChop >= 300 Then
//                    CreditChop = 300
//                    EndCredits = EndCredits + 1
//                    If EndCredits = 300 Then
//                        SetupCredits
//                        GameOutro = False
//                        GameMenu = True
//                    End If
//                Else
//                    EndCredits = 0
//                End If
//            ElseIf CreditChop > 100 And Credit(numCredits).Location.Y + Credit(numCredits).Location.Height > 0 Then
//                CreditChop = CreditChop - 0.02
//                If CreditChop < 100 Then CreditChop = 100
//            ElseIf musicPlaying = False Then
//                musicName = "tmusic"
//                ' mciSendString "play tmusic from 10", 0, 0, 0
//                PlayMusic "tmusic"
//                musicPlaying = True
//            End If
//        End With
//    Next A
//    If CreditChop <= 100 Or EndCredits > 0 Then
//        For A = 1 To 2
//            With Player(A).Controls
//                If (GetKeyState(vbKeyEscape) And KEY_PRESSED) Or (GetKeyState(vbKeySpace) And KEY_PRESSED) Or (GetKeyState(vbKeyReturn) And KEY_PRESSED) Then
//                    CreditChop = 300
//                    EndCredits = 0
//                    SetupCredits
//                    GameMenu = True
//                    GameOutro = False
//                End If
//            End With
//        Next A
//    End If
}

void DrawInterface(int Z, int numScreens)
{
//    Dim B As Integer
//    Dim C As Integer
//    Dim D As Integer
//    If ScreenType = 5 Or ScreenType = 6 Then   ' 2 Players
//        If numScreens = 1 And ScreenType <> 6 Then  'Only 1 screen
//            For B = 1 To numPlayers
//                If B = 1 Then
//                    C = -40
//                Else
//                    C = 40
//                End If
//                With Player(B)
//                    If .Character = 3 Or .Character = 4 Or .Character = 5 Then
//                        If B = 1 Then
//                            D = -1
//                        Else
//                            D = 1
//                        End If
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32 + 17 * D, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 17 * D, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32 + 17 * D, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                        If .Hearts > 0 Then
//                            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32 + 17 * D, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                        Else
//                            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32 + 17 * D, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                        End If
//                        If .Hearts > 1 Then
//                            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 17 * D, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                        Else
//                            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 17 * D, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                        End If
//                        If .Hearts > 2 Then
//                            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32 + 17 * D, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                        Else
//                            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32 + 17 * D, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                        End If
//                    Else
//    '2 players 1 screen heldbonus
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C, 16, GFX.Container(1).ScaleWidth, GFX.Container(1).ScaleHeight, GFX.ContainerMask(Player(B).Character).hdc, 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C, 16, GFX.Container(1).ScaleWidth, GFX.Container(1).ScaleHeight, GFX.Container(Player(B).Character).hdc, 0, 0, vbSrcPaint
//                        If .HeldBonus > 0 Then
//                            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C + 12, 16 + 12, NPCWidth(.HeldBonus), NPCHeight(.HeldBonus), GFXNPCMask(.HeldBonus), 0, 0, vbSrcAnd
//                            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C + 12, 16 + 12, NPCWidth(.HeldBonus), NPCHeight(.HeldBonus), GFXNPC(.HeldBonus), 0, 0, vbSrcPaint
//                        End If
//                    End If
//                End With
//            Next B

//            For B = 1 To 2
//                If B = 1 Then
//                    C = -58
//                Else
//                    C = 56
//                End If

//                If Player(B).Character = 5 And Player(B).Bombs > 0 Then
//                    BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 34 + C, 52, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(8).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 34 + C, 52, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(8).hdc, 0, 0, vbSrcPaint
//                    BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 10 + C, 53, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 10 + C, 53, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                    SuperPrint Str(Player(B).Bombs), 1, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 12 + C, 53
//                End If
//            Next B
//            C = 40

//            If BattleMode = False Then
//                'Print coins on the screen
//                If Player(1).HasKey Or Player(2).HasKey Then
//                    BitBlt myBackBuffer, -24 + 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(0).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, -24 + 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(0).hdc, 0, 0, vbSrcPaint
//                End If
//                If Player(1).Character = 5 Then
//                    BitBlt myBackBuffer, 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(6).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(6).hdc, 0, 0, vbSrcPaint
//                Else
//                    BitBlt myBackBuffer, 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(2).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(2).hdc, 0, 0, vbSrcPaint
//                End If
//                BitBlt myBackBuffer, 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96 + 8 + GFX.Interface(2).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96 + 8 + GFX.Interface(2).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                SuperPrint Str(Coins), 1, 40 + 20 - (Len(Str(Coins)) - 1) * 18 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface(3).ScaleWidth, 16 + 11
//                'Print Score
//                SuperPrint Str(Score), 1, 40 + 20 - (Len(Str(Score)) - 1) * 18 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 80 + 12 + 4 + 18 + 32 + GFX.Interface(3).ScaleWidth, 16 + 31
//                'Print lives on the screen
//                BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 - 16, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.InterfaceMask(3).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 - 16, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.Interface(3).hdc, 0, 0, vbSrcPaint
//                BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                SuperPrint Str(Lives), 1, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 12 + 18 + GFX.Interface(5).ScaleWidth, 16 + 11
//                'Print stars on the screen
//                If numStars > 0 Then
//                    BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122, 16 + 30, GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.InterfaceMask(5).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122, 16 + 30, GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.Interface(5).hdc, 0, 0, vbSrcPaint
//                    BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 31, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 31, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                    SuperPrint Str(numStars), 1, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 12 + 18 + GFX.Interface(5).ScaleWidth, 16 + 31
//                End If
//            Else
//            'plr 1 score
//                    BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 - 16, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.InterfaceMask(3).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 - 16, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.Interface(3).hdc, 0, 0, vbSrcPaint
//                    BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                    SuperPrint Str(BattleLives(1)), 1, -80 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 12 + 18 + GFX.Interface(5).ScaleWidth, 16 + 11
//            'plr 2 score
//                    BitBlt myBackBuffer, 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96 - 16, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.InterfaceMask(7).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96 - 16, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.Interface(7).hdc, 0, 0, vbSrcPaint
//                    BitBlt myBackBuffer, 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96 + 8 + GFX.Interface(2).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 40 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96 + 8 + GFX.Interface(2).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                    SuperPrint Str(BattleLives(2)), 1, 24 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface(3).ScaleWidth, 16 + 11
//            End If
//        Else 'Split screen


//'2 players 2 screen heldbonus

//            With Player(Z)
//                If .Character = 3 Or .Character = 4 Or .Character = 5 Then
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                    If .Hearts > 0 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                    End If
//                    If .Hearts > 1 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                    End If
//                    If .Hearts > 2 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                    End If
//                Else
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2, 16, GFX.Container(1).ScaleWidth + B, GFX.Container(1).ScaleHeight, GFX.ContainerMask(Player(Z).Character).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2, 16, GFX.Container(1).ScaleWidth + B, GFX.Container(1).ScaleHeight, GFX.Container(Player(Z).Character).hdc, 0, 0, vbSrcPaint
//                    If .HeldBonus > 0 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 12, 16 + 12, NPCWidth(.HeldBonus), NPCHeight(.HeldBonus), GFXNPCMask(.HeldBonus), 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 12, 16 + 12, NPCWidth(.HeldBonus), NPCHeight(.HeldBonus), GFXNPC(.HeldBonus), 0, 0, vbSrcPaint
//                    End If
//                End If
//            End With
//            If Player(Z).Character = 5 And Player(Z).Bombs > 0 Then
//                BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 34, 52, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(8).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 34, 52, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(8).hdc, 0, 0, vbSrcPaint
//                BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 10, 53, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 10, 53, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                SuperPrint Str(Player(Z).Bombs), 1, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 12, 53
//            End If
//            If BattleMode = False Then
//                'Print coins on the screen
//                If Player(Z).HasKey = True Then
//                    BitBlt myBackBuffer, -24 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(0).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, -24 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(0).hdc, 0, 0, vbSrcPaint
//                End If
//                If Player(Z).Character = 5 Then
//                    BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(6).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(6).hdc, 0, 0, vbSrcPaint
//                Else
//                    BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(2).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(2).hdc, 0, 0, vbSrcPaint
//                End If

//                BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96 + 8 + GFX.Interface(2).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96 + 8 + GFX.Interface(2).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                SuperPrint Str(Coins), 1, 20 - (Len(Str(Coins)) - 1) * 18 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface(3).ScaleWidth, 16 + 11
//                'Print Score
//                SuperPrint Str(Score), 1, 20 - (Len(Str(Score)) - 1) * 18 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 80 + 12 + 4 + 18 + 32 + GFX.Interface(3).ScaleWidth, 16 + 31
//                'Print lives on the screen
//                BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 - 16, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.InterfaceMask(3).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 - 16, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.Interface(3).hdc, 0, 0, vbSrcPaint
//                BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                SuperPrint Str(Lives), 1, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 12 + 18 + GFX.Interface(5).ScaleWidth, 16 + 11
//                'Print stars on the screen
//                If numStars > 0 Then
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122, 16 + 30, GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.InterfaceMask(5).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122, 16 + 30, GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.Interface(5).hdc, 0, 0, vbSrcPaint
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 31, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 31, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                    SuperPrint Str(numStars), 1, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 12 + 18 + GFX.Interface(5).ScaleWidth, 16 + 31
//                End If
//            Else
//                If Z = 1 Then
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.InterfaceMask(3).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.Interface(3).hdc, 0, 0, vbSrcPaint
//                Else
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.InterfaceMask(7).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.Interface(7).hdc, 0, 0, vbSrcPaint
//                End If
//                BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth + 16, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth + 16, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                SuperPrint Str(BattleLives(Z)), 1, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 12 + 18 + GFX.Interface(5).ScaleWidth + 16, 16 + 11
//            End If
//        End If
//    Else '1 Player or Multi Mario


//        If nPlay.Online = False Then
//            With Player(1)
//                If .Character = 3 Or .Character = 4 Or .Character = 5 Then
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                    If .Hearts > 0 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                    End If
//                    If .Hearts > 1 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                    End If
//                    If .Hearts > 2 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                    End If
//                Else
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2, 16, GFX.Container(1).ScaleWidth + B, GFX.Container(1).ScaleHeight, GFX.ContainerMask(0).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2, 16, GFX.Container(1).ScaleWidth + B, GFX.Container(1).ScaleHeight, GFX.Container(0).hdc, 0, 0, vbSrcPaint
//                    If .HeldBonus > 0 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 12, 16 + 12, NPCWidth(.HeldBonus), NPCHeight(.HeldBonus), GFXNPCMask(.HeldBonus), 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 12, 16 + 12, NPCWidth(.HeldBonus), NPCHeight(.HeldBonus), GFXNPC(.HeldBonus), 0, 0, vbSrcPaint
//                    End If
//                End If
//            End With
//        Else
//            With Player(nPlay.MySlot + 1)
//                If .Character = 3 Or .Character = 4 Then
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.HeartMask(1).hdc, 0, 0, vbSrcAnd
//                    If .Hearts > 0 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C - 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                    End If
//                    If .Hearts > 1 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                    End If
//                    If .Hearts > 2 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(1).hdc, 0, 0, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Heart(1).ScaleWidth / 2 + C + 32, 16, GFX.Heart(1).ScaleWidth, GFX.Heart(1).ScaleHeight, GFX.Heart(2).hdc, 0, 0, vbSrcPaint
//                    End If
//                Else
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2, 16, GFX.Container(1).ScaleWidth + B, GFX.Container(1).ScaleHeight, GFX.ContainerMask(0).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2, 16, GFX.Container(1).ScaleWidth + B, GFX.Container(1).ScaleHeight, GFX.Container(0).hdc, 0, 0, vbSrcPaint
//                    If .HeldBonus > 0 Then
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 12, 16 + 12, NPCWidth(.HeldBonus), NPCHeight(.HeldBonus), GFXNPCMask(.HeldBonus), 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 12, 16 + 12, NPCWidth(.HeldBonus), NPCHeight(.HeldBonus), GFXNPC(.HeldBonus), 0, 0, vbSrcPaint
//                    End If
//                End If
//            End With
//        End If
//        If Player(1).Character = 5 And Player(1).Bombs > 0 Then
//            BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 34 + C, 52, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(8).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 34 + C, 52, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(8).hdc, 0, 0, vbSrcPaint
//            BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 10 + C, 53, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 - 10 + C, 53, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//            SuperPrint Str(Player(1).Bombs), 1, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 12 + C, 53
//        End If
//        'Print coins on the screen
//        If Player(1).HasKey = True Then
//            BitBlt myBackBuffer, -24 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(0).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, -24 + 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(0).hdc, 0, 0, vbSrcPaint
//        End If
//        If Player(1).Character = 5 Then
//            BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(6).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(6).hdc, 0, 0, vbSrcPaint
//        Else
//            BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(2).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96, 16 + 10, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(2).hdc, 0, 0, vbSrcPaint
//        End If
//        BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96 + 8 + GFX.Interface(2).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//        BitBlt myBackBuffer, 20 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 96 + 8 + GFX.Interface(2).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//        SuperPrint Str(Coins), 1, 20 - (Len(Str(Coins)) - 1) * 18 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 80 + 4 + 12 + 18 + 32 + GFX.Interface(3).ScaleWidth, 16 + 11
//        'Print Score
//        SuperPrint Str(Score), 1, 20 - (Len(Str(Score)) - 1) * 18 + vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + 80 + 12 + 4 + 18 + 32 + GFX.Interface(3).ScaleWidth, 16 + 31
//        'Print lives on the screen
//        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 - 16, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.InterfaceMask(3).hdc, 0, 0, vbSrcAnd
//        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 - 16, 16 + 10, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.Interface(3).hdc, 0, 0, vbSrcPaint
//        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//        BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 11, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//        SuperPrint Str(Lives), 1, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 12 + 18 + GFX.Interface(5).ScaleWidth, 16 + 11
//        'Print stars on the screen
//        If numStars > 0 Then
//            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122, 16 + 30, GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.InterfaceMask(5).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122, 16 + 30, GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.Interface(5).hdc, 0, 0, vbSrcPaint
//            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 31, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 10 + GFX.Interface(1).ScaleWidth, 16 + 31, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//            SuperPrint Str(numStars), 1, vScreen(Z).Width / 2 - GFX.Container(1).ScaleWidth / 2 + C - 122 + 12 + 18 + GFX.Interface(5).ScaleWidth, 16 + 31
//        End If
//    End If
//    If BattleIntro > 0 Then
//        If BattleIntro > 45 Or BattleIntro Mod 2 = 1 Then
//            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.BMVs.ScaleWidth / 2, -96 + vScreen(Z).Height / 2 - GFX.BMVs.ScaleHeight / 2, GFX.BMVs.ScaleWidth, GFX.BMVs.ScaleHeight, GFX.BMVsMask.hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, vScreen(Z).Width / 2 - GFX.BMVs.ScaleWidth / 2, -96 + vScreen(Z).Height / 2 - GFX.BMVs.ScaleHeight / 2, GFX.BMVs.ScaleWidth, GFX.BMVs.ScaleHeight, GFX.BMVs.hdc, 0, 0, vbSrcPaint
//            BitBlt myBackBuffer, -50 + vScreen(Z).Width / 2 - GFX.CharacterName(Player(1).Character).ScaleWidth, -96 + vScreen(Z).Height / 2 - GFX.CharacterName(Player(1).Character).ScaleHeight / 2, GFX.CharacterName(Player(1).Character).ScaleWidth, GFX.CharacterName(Player(1).Character).ScaleHeight, GFX.CharacterNameMask(Player(1).Character).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, -50 + vScreen(Z).Width / 2 - GFX.CharacterName(Player(1).Character).ScaleWidth, -96 + vScreen(Z).Height / 2 - GFX.CharacterName(Player(1).Character).ScaleHeight / 2, GFX.CharacterName(Player(1).Character).ScaleWidth, GFX.CharacterName(Player(1).Character).ScaleHeight, GFX.CharacterName(Player(1).Character).hdc, 0, 0, vbSrcPaint
//            BitBlt myBackBuffer, 50 + vScreen(Z).Width / 2, -96 + vScreen(Z).Height / 2 - GFX.CharacterName(Player(2).Character).ScaleHeight / 2, GFX.CharacterName(Player(2).Character).ScaleWidth, GFX.CharacterName(Player(2).Character).ScaleHeight, GFX.CharacterNameMask(Player(2).Character).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, 50 + vScreen(Z).Width / 2, -96 + vScreen(Z).Height / 2 - GFX.CharacterName(Player(2).Character).ScaleHeight / 2, GFX.CharacterName(Player(2).Character).ScaleWidth, GFX.CharacterName(Player(2).Character).ScaleHeight, GFX.CharacterName(Player(2).Character).hdc, 0, 0, vbSrcPaint
//        End If
//    End If
//    If BattleOutro > 0 Then
//        BitBlt myBackBuffer, 10 + vScreen(Z).Width / 2, -96 + vScreen(Z).Height / 2 - GFX.BMWin.ScaleHeight / 2, GFX.BMWin.ScaleWidth, GFX.BMWin.ScaleHeight, GFX.BMWinMask.hdc, 0, 0, vbSrcAnd
//        BitBlt myBackBuffer, 10 + vScreen(Z).Width / 2, -96 + vScreen(Z).Height / 2 - GFX.BMWin.ScaleHeight / 2, GFX.BMWin.ScaleWidth, GFX.BMWin.ScaleHeight, GFX.BMWin.hdc, 0, 0, vbSrcPaint
//        BitBlt myBackBuffer, -10 + vScreen(Z).Width / 2 - GFX.CharacterName(Player(BattleWinner).Character).ScaleWidth, -96 + vScreen(Z).Height / 2 - GFX.CharacterName(Player(BattleWinner).Character).ScaleHeight / 2, GFX.CharacterName(Player(BattleWinner).Character).ScaleWidth, GFX.CharacterName(Player(BattleWinner).Character).ScaleHeight, GFX.CharacterNameMask(Player(BattleWinner).Character).hdc, 0, 0, vbSrcAnd
//        BitBlt myBackBuffer, -10 + vScreen(Z).Width / 2 - GFX.CharacterName(Player(BattleWinner).Character).ScaleWidth, -96 + vScreen(Z).Height / 2 - GFX.CharacterName(Player(BattleWinner).Character).ScaleHeight / 2, GFX.CharacterName(Player(BattleWinner).Character).ScaleWidth, GFX.CharacterName(Player(BattleWinner).Character).ScaleHeight, GFX.CharacterName(Player(BattleWinner).Character).hdc, 0, 0, vbSrcPaint
//    End If
}

int pfrX(int plrFrame)
{
//    Dim A As Integer
//        A = plrFrame
//        A = A - 50
//        Do While A > 100
//            A = A - 100
//        Loop
//        If A > 90 Then
//            A = 9
//        ElseIf A > 90 Then
//            A = 9
//        ElseIf A > 80 Then
//            A = 8
//        ElseIf A > 70 Then
//            A = 7
//        ElseIf A > 60 Then
//            A = 6
//        ElseIf A > 50 Then
//            A = 5
//        ElseIf A > 40 Then
//            A = 4
//        ElseIf A > 30 Then
//            A = 3
//        ElseIf A > 20 Then
//            A = 2
//        ElseIf A > 10 Then
//            A = 1
//        Else
//            A = 0
//        End If
//        pfrX = A * 100
}

int pfrY(int plrFrame)
{
//    Dim A As Integer
//    A = plrFrame
//    A = A - 50
//    Do While A > 100
//        A = A - 100
//    Loop
//    A = A - 1
//    Do While A > 9
//        A = A - 10
//    Loop
//    pfrY = A * 100
}

void GameThing()
{
//    Dim A As Integer
//    Dim B As Integer
//    Dim tempPlayer(1 To 2) As Player
//    tempPlayer(1) = Player(1)
//    tempPlayer(2) = Player(2)

//    SetupPlayers
//    Player(1).Location.SpeedY = 0
//    Player(2).Location.SpeedY = 0

//    If numPlayers = 1 Then
//        Player(1).Location.X = ScreenW / 2 - Player(1).Location.Width / 2
//        Player(1).Location.Y = ScreenH / 2 - Player(1).Location.Height + 24
//        Player(1).Direction = 1
//    Else
//        Player(1).Location.X = ScreenW / 2 - Player(1).Location.Width / 2 - 30
//        Player(1).Location.Y = ScreenH / 2 - Player(1).Location.Height + 24
//        Player(1).Direction = -1
//        Player(2).Location.X = ScreenW / 2 - Player(2).Location.Width / 2 + 32
//        Player(2).Location.Y = ScreenH / 2 - Player(2).Location.Height + 24
//        Player(2).Direction = 1
//    End If

//    PlayerFrame 1
//    PlayerFrame 2

//    BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness

//    For A = 1 To numPlayers
//        DrawPlayer A, 0
//    Next A

//    BitBlt myBackBuffer, ScreenW / 2 - 46, ScreenH / 2 + 31, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.InterfaceMask(3).hdc, 0, 0, vbSrcAnd
//    BitBlt myBackBuffer, ScreenW / 2 - 46, ScreenH / 2 + 31, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.Interface(3).hdc, 0, 0, vbSrcPaint
//    BitBlt myBackBuffer, ScreenW / 2 - GFX.Interface(1).ScaleWidth / 2, ScreenH / 2 + 32, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//    BitBlt myBackBuffer, ScreenW / 2 - GFX.Interface(1).ScaleWidth / 2, ScreenH / 2 + 32, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//    SuperPrint Str(Lives), 1, ScreenW / 2 + 12, ScreenH / 2 + 32

//    StretchBlt frmMain.hdc, 0, 0, frmMain.ScaleWidth, frmMain.ScaleHeight, myBackBuffer, 0, 0, 800, 600, vbSrcCopy

//    Player(1) = tempPlayer(1)
//    Player(2) = tempPlayer(2)
//    DoEvents
}

void DrawPlayer(int A, int Z)
{
//    On Error Resume Next
//    Dim B As Integer
//    Dim C As Double
//    With Player(A)
//        If .Dead = False And .Immune2 = False And .TimeToLive = 0 And Not (.Effect = 3 Or .Effect = 5 Or .Effect = 8 Or .Effect = 10) Then
//            If vScreenCollision(Z, .Location) Then
//                If .Mount = 3 And .Fairy = False Then
//                    B = .MountType
//                    'Yoshi's Tongue
//                    If .MountSpecial > 0 Then

//                        C = 0
//                        If .Direction = -1 Then C = .YoshiTongueLength

//                        BitBlt myBackBuffer, vScreenX(Z) + .YoshiTongueX - C - 1, vScreenY(Z) + .YoshiTongue.Y, .YoshiTongueLength + 2, 16, GFX.TongueMask(2).hdc, 0, 0, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + .YoshiTongueX - C - 1, vScreenY(Z) + .YoshiTongue.Y, .YoshiTongueLength + 2, 16, GFX.Tongue(2).hdc, 0, 0, vbSrcPaint

//                        C = 1
//                        If .Direction = 1 Then C = 0

//                        BitBlt myBackBuffer, vScreenX(Z) + Int(.YoshiTongue.X), vScreenY(Z) + .YoshiTongue.Y, 16, 16, GFX.TongueMask(1).hdc, 0, 16 * C, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.YoshiTongue.X), vScreenY(Z) + .YoshiTongue.Y, 16, 16, GFX.Tongue(1).hdc, 0, 16 * C, vbSrcPaint

//                    End If
//                    'Yoshi's Body
//                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .YoshiBX, vScreenY(Z) + .Location.Y + .YoshiBY, 32, 32, GFXYoshiBMask(B), 0, 32 * .YoshiBFrame, vbSrcAnd
//                    If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .YoshiBX, vScreenY(Z) + .Location.Y + .YoshiBY, 32, 32, GFXYoshiB(B), 0, 32 * .YoshiBFrame, vbSrcPaint
//                    'Yoshi's Head
//                    BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .YoshiTX, vScreenY(Z) + .Location.Y + .YoshiTY, 32, 32, GFXYoshiTMask(B), 0, 32 * .YoshiTFrame, vbSrcAnd
//                    If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .YoshiTX, vScreenY(Z) + .Location.Y + .YoshiTY, 32, 32, GFXYoshiT(B), 0, 32 * .YoshiTFrame, vbSrcPaint
//                End If
//                If .Fairy = True Then 'draw a fairy
//                    .Frame = 1
//                    If .Immune2 = False Then
//                    If .Direction = 1 Then
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 5, vScreenY(Z) + .Location.Y - 2, 32, 32, GFXNPCMask(254), 0, (SpecialFrame(9) + 1) * 32, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 5, vScreenY(Z) + .Location.Y - 2, 32, 32, GFXNPC(254), 0, (SpecialFrame(9) + 1) * 32, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 5, vScreenY(Z) + .Location.Y - 2, 32, 32, GFXNPCMask(254), 0, (SpecialFrame(9) + 3) * 32, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X - 5, vScreenY(Z) + .Location.Y - 2, 32, 32, GFXNPC(254), 0, (SpecialFrame(9) + 3) * 32, vbSrcPaint
//                    End If
//                    End If
//                ElseIf .Character = 1 Then 'draw mario
//                    If .Mount = 0 Then
//                        BitBlt myBackBuffer, _
//                            vScreenX(Z) + Int(.Location.X) + MarioFrameX((.State * 100) + (.Frame * .Direction)), _
//                            vScreenY(Z) + .Location.Y + MarioFrameY((.State * 100) + (.Frame * .Direction)), _
//                            99, 99, _
//                            GFXMarioMask(.State), _
//                            pfrX(100 + .Frame * .Direction), _
//                            pfrY(100 + .Frame * .Direction), _
//                            vbSrcAnd

//                        If ShadowMode = False Then _
//                            BitBlt myBackBuffer, _
//                                vScreenX(Z) + Int(.Location.X) + MarioFrameX((.State * 100) + (.Frame * .Direction)), _
//                                vScreenY(Z) + .Location.Y + MarioFrameY((.State * 100) + (.Frame * .Direction)), _
//                                99, 99, _
//                                GFXMario(.State), _
//                                pfrX(100 + .Frame * .Direction), _
//                                pfrY(100 + .Frame * .Direction), _
//                                vbSrcPaint
//                    ElseIf .Mount = 1 Then
//                        If .Duck = False Then
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + MarioFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + MarioFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26 - .MountOffsetY - MarioFrameY((.State * 100) + (.Frame * .Direction)), GFXMarioMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + MarioFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + MarioFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26 - .MountOffsetY - MarioFrameY((.State * 100) + (.Frame * .Direction)), GFXMario(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                        End If
//                        BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 16, vScreenY(Z) + .Location.Y + .Location.Height - 30, 32, 32, GFX.BootMask(.MountType).hdc, 0, 32 * .MountFrame, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 16, vScreenY(Z) + .Location.Y + .Location.Height - 30, 32, 32, GFX.Boot(.MountType).hdc, 0, 32 * .MountFrame, vbSrcPaint
//                    ElseIf .Mount = 3 Then
//                        BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + MarioFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + MarioFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXMarioMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + MarioFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + MarioFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXMario(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                    End If
//                ElseIf .Character = 2 Then 'draw luigi
//                    If .Mount = 0 Then
//                        BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + LuigiFrameY((.State * 100) + (.Frame * .Direction)), 99, 99, GFXLuigiMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + LuigiFrameY((.State * 100) + (.Frame * .Direction)), 99, 99, GFXLuigi(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                    ElseIf .Mount = 1 Then
//                        If .Duck = False Then
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + LuigiFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26 - .MountOffsetY - LuigiFrameY((.State * 100) + (.Frame * .Direction)), GFXLuigiMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + LuigiFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26 - .MountOffsetY - LuigiFrameY((.State * 100) + (.Frame * .Direction)), GFXLuigi(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                        End If
//                        BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 16, vScreenY(Z) + .Location.Y + .Location.Height - 30, 32, 32, GFX.BootMask(.MountType).hdc, 0, 32 * .MountFrame, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 16, vScreenY(Z) + .Location.Y + .Location.Height - 30, 32, 32, GFX.Boot(.MountType).hdc, 0, 32 * .MountFrame, vbSrcPaint
//                    ElseIf .Mount = 3 Then
//                        BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + LuigiFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXLuigiMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + LuigiFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXLuigi(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                    End If
//                ElseIf .Character = 3 Then 'draw peach
//                    If .Mount = 0 Then
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + PeachFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + PeachFrameY((.State * 100) + (.Frame * .Direction)), 99, 99, GFXPeachMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + PeachFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + PeachFrameY((.State * 100) + (.Frame * .Direction)), 99, 99, GFXPeach(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                    ElseIf .Mount = 1 Then
//                        If .Duck = False Then
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + PeachFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + PeachFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26 - PeachFrameY((.State * 100) + (.Frame * .Direction)) - 2, GFXPeachMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + PeachFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + PeachFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26 - PeachFrameY((.State * 100) + (.Frame * .Direction)) - 2, GFXPeach(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                        End If
//                        BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 16, vScreenY(Z) + .Location.Y + .Location.Height - 30, 32, 32, GFX.BootMask(.MountType).hdc, 0, 32 * .MountFrame, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 16, vScreenY(Z) + .Location.Y + .Location.Height - 30, 32, 32, GFX.Boot(.MountType).hdc, 0, 32 * .MountFrame, vbSrcPaint
//                    End If
//                ElseIf .Character = 4 Then 'draw Toad
//                    If .Mount = 0 Then
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + ToadFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)), 99, 99, GFXToadMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + ToadFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)), 99, 99, GFXToad(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                    ElseIf .Mount = 1 Then
//                        If .Duck = False Then
//                            If .State = 1 Then
//                                BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + ToadFrameX((.State * 100) + (.Frame * .Direction)), 6 + vScreenY(Z) + .Location.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26 - .MountOffsetY - ToadFrameY((.State * 100) + (.Frame * .Direction)), GFXToadMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                                If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + ToadFrameX((.State * 100) + (.Frame * .Direction)), 6 + vScreenY(Z) + .Location.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26 - .MountOffsetY - ToadFrameY((.State * 100) + (.Frame * .Direction)), GFXToad(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                            Else
//                                BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + ToadFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26 - .MountOffsetY - ToadFrameY((.State * 100) + (.Frame * .Direction)), GFXToadMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                                If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + ToadFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26 - .MountOffsetY - ToadFrameY((.State * 100) + (.Frame * .Direction)), GFXToad(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                            End If
//                        End If
//                        BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 16, vScreenY(Z) + .Location.Y + .Location.Height - 30, 32, 32, GFX.BootMask(.MountType).hdc, 0, 32 * .MountFrame, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 16, vScreenY(Z) + .Location.Y + .Location.Height - 30, 32, 32, GFX.Boot(.MountType).hdc, 0, 32 * .MountFrame, vbSrcPaint
//                    End If
//                ElseIf .Character = 5 Then 'draw link
//                    If .Mount = 0 Then
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + LinkFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + LinkFrameY((.State * 100) + (.Frame * .Direction)), 99, 99, GFXLinkMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + LinkFrameX((.State * 100) + (.Frame * .Direction)), vScreenY(Z) + .Location.Y + LinkFrameY((.State * 100) + (.Frame * .Direction)), 99, 99, GFXLink(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                    End If
//                End If
//            'peach/toad held npcs
//                If (.Character = 3 Or .Character = 4) And .HoldingNPC > 0 And .Effect <> 7 Then
//                    With NPC(.HoldingNPC)
//                        If .Type <> 263 Then
//                            If (((.HoldingPlayer > 0 And Player(.HoldingPlayer).Effect <> 3) Or (.Type = 50 And .standingOnPlayer = 0) Or .Type = 17 And .CantHurt > 0) Or .Effect = 5) And Not .Type = 91 And Player(.HoldingPlayer).Dead = False Then
//                                If NPCIsYoshi(.Type) = False And .Type > 0 Then
//                                    If NPCWidthGFX(.Type) = 0 Then
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * .Location.Height, vbSrcAnd
//                                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * .Location.Height, vbSrcPaint
//                                    Else
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + (NPCFrameOffsetX(.Type) * -.Direction) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPCMask(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcAnd
//                                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + (NPCFrameOffsetX(.Type) * -.Direction) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPC(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcPaint
//                                    End If
//                                End If
//                            End If
//                        Else
//                            DrawFrozenNPC Z, Player(A).HoldingNPC
//                        End If
//                    End With
//                End If
//                If .Fairy = False Then
//                    If .Mount = 3 And .YoshiBlue = True Then
//                        If .Direction = 1 Then
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .YoshiBX - 12, vScreenY(Z) + .Location.Y + .YoshiBY - 16, 32, 32, GFX.YoshiWingsMask.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .YoshiBX - 12, vScreenY(Z) + .Location.Y + .YoshiBY - 16, 32, 32, GFX.YoshiWings.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcPaint
//                        Else
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .YoshiBX + 12, vScreenY(Z) + .Location.Y + .YoshiBY - 16, 32, 32, GFX.YoshiWingsMask.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .YoshiBX + 12, vScreenY(Z) + .Location.Y + .YoshiBY - 16, 32, 32, GFX.YoshiWings.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcPaint
//                        End If
//                    End If
//                    If .Mount = 1 And .MountType = 3 Then
//                        If .Direction = 1 Then
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) - 24, vScreenY(Z) + .Location.Y + .Location.Height - 40, 32, 32, GFX.YoshiWingsMask.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) - 24, vScreenY(Z) + .Location.Y + .Location.Height - 40, 32, 32, GFX.YoshiWings.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcPaint
//                        Else
//                            BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + 16, vScreenY(Z) + .Location.Y + .Location.Height - 40, 32, 32, GFX.YoshiWingsMask.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + 16, vScreenY(Z) + .Location.Y + .Location.Height - 40, 32, 32, GFX.YoshiWings.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcPaint
//                        End If
//                    End If
//                End If
//            End If
//        End If
//    End With
}

void ScreenShot()
{
    frmMain.makeShot();
    PlaySound(12);
    TakeScreen = false;
}

void DrawFrozenNPC(int Z, int A)
{
//    With NPC(A)
    auto &n = NPC[A];
//        If (vScreenCollision(Z, .Location) Or vScreenCollision(Z, newLoc(.Location.X - (NPCWidthGFX(.Type) - .Location.Width) / 2, .Location.Y, CDbl(NPCWidthGFX(.Type)), CDbl(NPCHeight(.Type))))) And .Hidden = False Then
    if((vScreenCollision(Z, n.Location) Or
        vScreenCollision(Z, newLoc(n.Location.X - (NPCWidthGFX[n.Type] - n.Location.Width) / 2,
                            n.Location.Y, CDbl(NPCWidthGFX[n.Type]), CDbl(NPCHeight[n.Type])))) And !n.Hidden)
    {
//        'draw npc

//            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + 2, vScreenY(Z) + .Location.Y + 2, .Location.Width - 4, .Location.Height - 4, GFXNPCMask(.Special), 2, 2 + .Special2 * NPCHeight(.Special), vbSrcAnd
//            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + 2, vScreenY(Z) + .Location.Y + 2, .Location.Width - 4, .Location.Height - 4, GFXNPC(.Special), 2, 2 + .Special2 * NPCHeight(.Special), vbSrcPaint
        float c = n.Shadow ? 0.f : 1.f;

        frmMain.renderTexture(int(vScreenX[Z] + n.Location.X + 2),
                              int(vScreenY[Z] + n.Location.Y + 2),
                              int(n.Location.Width - 4),
                              int(n.Location.Height - 4),
                              GFXNPCBMP[int(n.Special)],
                              2, 2 + int(n.Special2) * NPCHeight[int(n.Special)], c, c, c);

//        'draw ice
//            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width - 6, .Location.Height - 6, GFXNPCMask(.Type), 0, 0, vbSrcAnd
//            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width - 6, .Location.Height - 6, GFXNPC(.Type), 0, 0, vbSrcPaint
        frmMain.renderTexture(int(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type]),
                              int(vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type]),
                              int(n.Location.Width - 6), int(n.Location.Height - 6),
                              GFXNPCBMP[n.Type], 0, 0, c, c, c);
//            BitBlt myBackBuffer, vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] + n.Location.Width - 6, vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type], 6, n.Location.Height - 6, GFXNPCMask[n.Type], 128 - 6, 0, vbSrcAnd
//            If .Shadow = False Then BitBlt myBackBuffer, vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] + n.Location.Width - 6, vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type], 6, n.Location.Height - 6, GFXNPC[n.Type], 128 - 6, 0, vbSrcPaint
        frmMain.renderTexture(int(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] + n.Location.Width - 6),
                              int(vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type]),
                              6, int(n.Location.Height - 6),
                              GFXNPCBMP[n.Type], 128 - 6, 0, c, c, c);
//            BitBlt myBackBuffer, vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type], vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] + n.Location.Height - 6, n.Location.Width - 6, 6, GFXNPCMask[n.Type], 0, 128 - 6, vbSrcAnd
//            If .Shadow = False Then BitBlt myBackBuffer, vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type], vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] + n.Location.Height - 6, n.Location.Width - 6, 6, GFXNPC[n.Type], 0, 128 - 6, vbSrcPaint
        frmMain.renderTexture(int(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type]),
                              int(vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] + n.Location.Height - 6),
                              int(n.Location.Width - 6), 6,
                              GFXNPCBMP[n.Type], 0, 128 - 6, c, c, c);
//            BitBlt myBackBuffer, vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] + n.Location.Width - 6, vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] + n.Location.Height - 6, 6, 6, GFXNPCMask[n.Type], 128 - 6, 128 - 6, vbSrcAnd
//            If .Shadow = False Then BitBlt myBackBuffer, vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] + n.Location.Width - 6, vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] + n.Location.Height - 6, 6, 6, GFXNPC[n.Type], 128 - 6, 128 - 6, vbSrcPaint
        frmMain.renderTexture(int(vScreenX[Z] + n.Location.X + NPCFrameOffsetX[n.Type] + n.Location.Width - 6),
                              int(vScreenY[Z] + n.Location.Y + NPCFrameOffsetY[n.Type] + n.Location.Height - 6),
                              6, 6, GFXNPCBMP[n.Type],
                              128 - 6, 128 - 6);
//        End If
    }
//    End With
}
