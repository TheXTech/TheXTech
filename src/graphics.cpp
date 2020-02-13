#include "globals.h"
#include "graphics.h"
#include "collision.h"
#include "game_main.h"
#include "gfx.h"
#include "sound.h"
#include "change_res.h"
#include <fmt_format_ne.h>

#include "pseudo_vb.h"

void SpecialFrames();

// draws GFX to screen when on the world map/world map editor
void UpdateGraphics2()
{
//    cycleCount = cycleCount + 1
//    If FrameSkip = True Then
//        If GetTickCount + Int(1000 * (1 - (cycleCount / 63))) > GoalTime Then
//            Exit Sub
//        End If
//    End If
//    fpsCount = fpsCount + 1
//    Dim A As Integer
//    Dim B As Integer
//    Dim Z As Integer
//    Dim WPHeight As Integer
//    Dim tempLocation As Location
//    Z = 1
//    vScreen(Z).Left = 0
//    vScreen(Z).Top = 0
//    vScreen(Z).Width = ScreenW
//    vScreen(Z).Height = ScreenH
//    SpecialFrames
//    SceneFrame2(1) = SceneFrame2(1) + 1
//    If SceneFrame2(1) >= 12 Then
//        SceneFrame2(1) = 0
//        SceneFrame(1) = SceneFrame(1) + 1
//        If SceneFrame(1) >= 4 Then SceneFrame(1) = 0
//        SceneFrame(4) = SceneFrame(1)
//        SceneFrame(5) = SceneFrame(1)
//        SceneFrame(6) = SceneFrame(1)
//        SceneFrame(9) = SceneFrame(1)
//        SceneFrame(10) = SceneFrame(1)
//        SceneFrame(12) = SceneFrame(1)
//        SceneFrame(51) = SceneFrame(1)
//        SceneFrame(52) = SceneFrame(1)
//        SceneFrame(53) = SceneFrame(1)
//        SceneFrame(54) = SceneFrame(1)
//        SceneFrame(55) = SceneFrame(1)
//    End If
//    SceneFrame2(27) = SceneFrame2(27) + 1
//    If SceneFrame2(27) >= 8 Then
//        SceneFrame2(27) = 0
//        SceneFrame(27) = SceneFrame(27) + 1
//        If SceneFrame(27) >= 12 Then SceneFrame(27) = 0
//        SceneFrame(28) = SceneFrame(27)
//        SceneFrame(29) = SceneFrame(27)
//        SceneFrame(30) = SceneFrame(27)
//    End If
//    SceneFrame2(33) = SceneFrame2(33) + 1
//    If SceneFrame2(33) >= 4 Then
//        SceneFrame2(33) = 0
//        SceneFrame(33) = SceneFrame(27) + 1
//        If SceneFrame(33) >= 14 Then SceneFrame(33) = 0
//        SceneFrame(34) = SceneFrame(33)
//    End If
//    SceneFrame2(62) = SceneFrame2(62) + 1
//    If SceneFrame2(62) >= 8 Then
//        SceneFrame2(62) = 0
//        SceneFrame(62) = SceneFrame(62) + 1
//        If SceneFrame(62) >= 8 Then SceneFrame(62) = 0
//        SceneFrame(63) = SceneFrame(62)
//    End If
//    LevelFrame2(2) = LevelFrame2(2) + 1
//    If LevelFrame2(2) >= 6 Then
//        LevelFrame2(2) = 0
//        LevelFrame(2) = LevelFrame(2) + 1
//        If LevelFrame(2) >= 6 Then LevelFrame(2) = 0
//        LevelFrame(9) = LevelFrame(2)
//        LevelFrame(13) = LevelFrame(2)
//        LevelFrame(14) = LevelFrame(2)
//        LevelFrame(15) = LevelFrame(2)
//        LevelFrame(31) = LevelFrame(2)
//        LevelFrame(32) = LevelFrame(2)
//    End If
//    LevelFrame2(8) = LevelFrame2(8) + 1
//    If LevelFrame2(8) >= 12 Then
//        LevelFrame2(8) = 0
//        LevelFrame(8) = LevelFrame(8) + 1
//        If LevelFrame(8) >= 4 Then LevelFrame(8) = 0
//    End If
//    LevelFrame2(12) = LevelFrame2(12) + 1
//    If LevelFrame2(12) >= 8 Then
//        LevelFrame2(12) = 0
//        LevelFrame(12) = LevelFrame(12) + 1
//        If LevelFrame(12) >= 2 Then LevelFrame(12) = 0
//    End If
//    LevelFrame2(25) = LevelFrame2(25) + 1
//    If LevelFrame2(25) >= 8 Then
//        LevelFrame2(25) = 0
//        LevelFrame(25) = LevelFrame(25) + 1
//        If LevelFrame(25) >= 4 Then LevelFrame(25) = 0
//        LevelFrame(26) = LevelFrame(25)
//    End If
//    TileFrame2(14) = TileFrame2(14) + 1
//    If TileFrame2(14) >= 14 Then
//        TileFrame2(14) = 0
//        TileFrame(14) = TileFrame(14) + 1
//        If TileFrame(14) >= 4 Then TileFrame(14) = 0
//        TileFrame(27) = TileFrame(14)
//        TileFrame(241) = TileFrame(14)
//    End If
//    If WorldEditor = True Then
//        BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness
//    Else
//        BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness
//    End If
//    If TakeScreen = True Then
//        If LevelEditor = True Or MagicHand = True Then
//            frmLevelWindow.vScreen(1).AutoRedraw = True
//        Else
//            frmMain.AutoRedraw = True
//        End If
//    End If
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
//        For A = 1 To numTiles
//            With Tile(A)
//                If vScreenCollision2(1, .Location) = True Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXTile(.Type), 0, TileHeight(.Type) * TileFrame(.Type), vbSrcCopy
//                End If
//            End With
//        Next A
//        For A = 1 To numScenes
//            With Scene(A)
//                If vScreenCollision2(1, .Location) = True And .Active = True Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXSceneMask(.Type), 0, SceneHeight(.Type) * SceneFrame(.Type), vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXScene(.Type), 0, SceneHeight(.Type) * SceneFrame(.Type), vbSrcPaint
//                End If
//            End With
//        Next A
//        For A = 1 To numWorldPaths
//            With WorldPath(A)
//                If vScreenCollision2(1, .Location) = True And .Active = True Then
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXPathMask(.Type), 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXPath(.Type), 0, 0, vbSrcPaint
//                End If
//            End With
//        Next A
//        For A = 1 To numWorldLevels
//            With WorldLevel(A)
//                If vScreenCollision2(1, .Location) = True And .Active = True Then
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
//    End If
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
//        With WorldPlayer(1)
//            If .Type = 0 Then .Type = 1
//            If Player(1).Character = 1 Then .Type = 1
//            If Player(1).Character = 2 Then .Type = 2
//            If Player(1).Character = 3 Then .Type = 3
//            If Player(1).Character = 4 Then .Type = 4
//            If Player(1).Character = 5 Then .Type = 5
//            If .Type = 3 Then
//                WPHeight = 44
//            ElseIf .Type = 4 Then
//                WPHeight = 40
//            Else
//                WPHeight = 32
//            End If
//            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y - 10 + .Location.Height - WPHeight, .Location.Width, WPHeight, GFXPlayerMask(.Type), 0, WPHeight * .Frame, vbSrcAnd
//            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y - 10 + .Location.Height - WPHeight, .Location.Width, WPHeight, GFXPlayer(.Type), 0, WPHeight * .Frame, vbSrcPaint
//        End With
//        BitBlt myBackBuffer, 0, 0, 800, 130, GFX.Interface(4).hdc, 0, 0, vbSrcCopy
//        BitBlt myBackBuffer, 0, 534, 800, 66, GFX.Interface(4).hdc, 0, 534, vbSrcCopy
//        BitBlt myBackBuffer, 0, 130, 66, 404, GFX.Interface(4).hdc, 0, 130, vbSrcCopy
//        BitBlt myBackBuffer, 734, 130, 66, 404, GFX.Interface(4).hdc, 734, 130, vbSrcCopy
//        For A = 1 To numPlayers
//            With Player(A)
//                .Direction = -1
//                .Location.SpeedY = 0
//                .Location.SpeedX = -1
//                .Controls.Left = False
//                .Controls.Right = False
//                If .Duck = True Then UnDuck A
//                PlayerFrame A
//                If .Mount = 3 Then
//                    If .MountType = 0 Then .MountType = 1
//                    B = .MountType
//                    'Yoshi's Body
//                    BitBlt myBackBuffer, 32 + (48 * A) + .YoshiBX, 124 - .Location.Height + .YoshiBY, 32, 32, GFXYoshiBMask(B), 0, 32 * .YoshiBFrame, vbSrcAnd
//                    If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + .YoshiBX, 124 - .Location.Height + .YoshiBY, 32, 32, GFXYoshiB(B), 0, 32 * .YoshiBFrame, vbSrcPaint
//                    'Yoshi's Head
//                    BitBlt myBackBuffer, 32 + (48 * A) + .YoshiTX, 124 - .Location.Height + .YoshiTY, 32, 32, GFXYoshiTMask(B), 0, 32 * .YoshiTFrame, vbSrcAnd
//                    If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + .YoshiTX, 124 - .Location.Height + .YoshiTY, 32, 32, GFXYoshiT(B), 0, 32 * .YoshiTFrame, vbSrcPaint
//                End If
//                If .Character = 1 Then
//                    If .Mount = 0 Or .Mount = 3 Then
//                        BitBlt myBackBuffer, 32 + (48 * A) + MarioFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + MarioFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXMarioMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + MarioFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + MarioFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXMario(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                    ElseIf .Mount = 1 Then
//                        BitBlt myBackBuffer, 32 + (48 * A) + MarioFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + MarioFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26, GFXMarioMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + MarioFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + MarioFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 26, GFXMario(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                        BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16, 124 - 30, 32, 32, GFX.BootMask(.MountType).hdc, 0, 32 * .MountFrame, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16, 124 - 30, 32, 32, GFX.Boot(.MountType).hdc, 0, 32 * .MountFrame, vbSrcPaint
//                        If .MountType = 3 Then
//                            .YoshiWingsFrameCount = .YoshiWingsFrameCount + 1
//                            .YoshiWingsFrame = 0
//                            If .YoshiWingsFrameCount <= 12 Then
//                                .YoshiWingsFrame = 1
//                            ElseIf .YoshiWingsFrameCount >= 24 Then
//                                .YoshiWingsFrameCount = 0
//                            End If
//                            BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWingsMask.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWings.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcPaint
//                        End If
//                    End If
//                ElseIf .Character = 2 Then
//                    If .Mount = 0 Or .Mount = 3 Then
//                        BitBlt myBackBuffer, 32 + (48 * A) + LuigiFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + LuigiFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXLuigiMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + LuigiFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + LuigiFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXLuigi(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                    ElseIf .Mount = 1 Then
//                        BitBlt myBackBuffer, 32 + (48 * A) + LuigiFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + LuigiFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 24, GFXLuigiMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + LuigiFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + LuigiFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 24, GFXLuigi(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                        BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16, 124 - 30, 32, 32, GFX.BootMask(.MountType).hdc, 0, 32 * .MountFrame, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16, 124 - 30, 32, 32, GFX.Boot(.MountType).hdc, 0, 32 * .MountFrame, vbSrcPaint
//                        If .MountType = 3 Then
//                            .YoshiWingsFrameCount = .YoshiWingsFrameCount + 1
//                            .YoshiWingsFrame = 0
//                            If .YoshiWingsFrameCount <= 12 Then
//                                .YoshiWingsFrame = 1
//                            ElseIf .YoshiWingsFrameCount >= 24 Then
//                                .YoshiWingsFrameCount = 0
//                            End If
//                            BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWingsMask.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWings.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcPaint
//                        End If
//                    End If
//                ElseIf .Character = 3 Then
//                    If .Mount = 0 Or .Mount = 3 Then
//                        BitBlt myBackBuffer, 32 + (48 * A) + PeachFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + PeachFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXPeachMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + PeachFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + PeachFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXPeach(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                    ElseIf .Mount = 1 Then

//                        BitBlt myBackBuffer, 32 + (48 * A) + PeachFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + PeachFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 24, GFXPeachMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + PeachFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + PeachFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 24, GFXPeach(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint

//                        BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16, 124 - 30, 32, 32, GFX.BootMask(.MountType).hdc, 0, 32 * .MountFrame, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16, 124 - 30, 32, 32, GFX.Boot(.MountType).hdc, 0, 32 * .MountFrame, vbSrcPaint
//                        If .MountType = 3 Then
//                            .YoshiWingsFrameCount = .YoshiWingsFrameCount + 1
//                            .YoshiWingsFrame = 0
//                            If .YoshiWingsFrameCount <= 12 Then
//                                .YoshiWingsFrame = 1
//                            ElseIf .YoshiWingsFrameCount >= 24 Then
//                                .YoshiWingsFrameCount = 0
//                            End If
//                            BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWingsMask.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWings.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcPaint
//                        End If
//                    End If
//                ElseIf .Character = 4 Then
//                    If .Mount = 0 Or .Mount = 3 Then
//                        BitBlt myBackBuffer, 32 + (48 * A) + ToadFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + ToadFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXToadMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + ToadFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + ToadFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY, 99, 99, GFXToad(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                    ElseIf .Mount = 1 Then
//                        If .State = 1 Then
//                            BitBlt myBackBuffer, 32 + (48 * A) + ToadFrameX((.State * 100) + (.Frame * .Direction)), 6 + 124 - .Location.Height + ToadFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 24, GFXToadMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + ToadFrameX((.State * 100) + (.Frame * .Direction)), 6 + 124 - .Location.Height + ToadFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 24, GFXToad(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                        Else
//                            BitBlt myBackBuffer, 32 + (48 * A) + ToadFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + ToadFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 24, GFXToadMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + ToadFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + ToadFrameY((.State * 100) + (.Frame * .Direction)), 99, .Location.Height - 24, GFXToad(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                        End If
//                        BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16, 124 - 30, 32, 32, GFX.BootMask(.MountType).hdc, 0, 32 * .MountFrame, vbSrcAnd
//                        If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16, 124 - 30, 32, 32, GFX.Boot(.MountType).hdc, 0, 32 * .MountFrame, vbSrcPaint

//                        If .MountType = 3 Then
//                            .YoshiWingsFrameCount = .YoshiWingsFrameCount + 1
//                            .YoshiWingsFrame = 0
//                            If .YoshiWingsFrameCount <= 12 Then
//                                .YoshiWingsFrame = 1
//                            ElseIf .YoshiWingsFrameCount >= 24 Then
//                                .YoshiWingsFrameCount = 0
//                            End If
//                            BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWingsMask.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + .Location.Width / 2 - 16 + 20, 124 - 30 - 10, 32, 32, GFX.YoshiWings.hdc, 0, 0 + 32 * .YoshiWingsFrame, vbSrcPaint
//                        End If
//                    End If
//                ElseIf .Character = 5 Then
//                    BitBlt myBackBuffer, 32 + (48 * A) + LinkFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + LinkFrameY((.State * 100) + (.Frame * .Direction)), 99, 99, GFXLinkMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                    If ShadowMode = False Then BitBlt myBackBuffer, 32 + (48 * A) + LinkFrameX((.State * 100) + (.Frame * .Direction)), 124 - .Location.Height + LinkFrameY((.State * 100) + (.Frame * .Direction)), 99, 99, GFXLink(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                End If
//            End With
//        Next A
//        A = numPlayers + 1
//        'Print lives on the screen
//        BitBlt myBackBuffer, 32 + (48 * A), 126 - GFX.Interface(3).ScaleHeight, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.InterfaceMask(3).hdc, 0, 0, vbSrcAnd
//        BitBlt myBackBuffer, 32 + (48 * A), 126 - GFX.Interface(3).ScaleHeight, GFX.Interface(3).ScaleWidth, GFX.Interface(3).ScaleHeight, GFX.Interface(3).hdc, 0, 0, vbSrcPaint
//        BitBlt myBackBuffer, 32 + (48 * A) + 40, 128 - GFX.Interface(3).ScaleHeight, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//        BitBlt myBackBuffer, 32 + (48 * A) + 40, 128 - GFX.Interface(3).ScaleHeight, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//        SuperPrint Str(Lives), 1, 32 + (48 * A) + 62, 112
//        'Print coins on the screen
//        If Player(1).Character = 5 Then
//            BitBlt myBackBuffer, 32 + (48 * A) + 16, 88, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(6).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, 32 + (48 * A) + 16, 88, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(6).hdc, 0, 0, vbSrcPaint
//        Else
//            BitBlt myBackBuffer, 32 + (48 * A) + 16, 88, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.InterfaceMask(2).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, 32 + (48 * A) + 16, 88, GFX.Interface(2).ScaleWidth, GFX.Interface(2).ScaleHeight, GFX.Interface(2).hdc, 0, 0, vbSrcPaint
//        End If
//        BitBlt myBackBuffer, 32 + (48 * A) + 40, 90, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//        BitBlt myBackBuffer, 32 + (48 * A) + 40, 90, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//        SuperPrint Str(Coins), 1, 32 + (48 * A) + 62, 90
//        'Print stars on the screen
//        If numStars > 0 Then
//            BitBlt myBackBuffer, 32 + (48 * A) + 16, 66, GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.InterfaceMask(5).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, 32 + (48 * A) + 16, 66, GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.Interface(5).hdc, 0, 0, vbSrcPaint
//            BitBlt myBackBuffer, 32 + (48 * A) + 40, 68, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, 32 + (48 * A) + 40, 68, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//            SuperPrint Str(numStars), 1, 32 + (48 * A) + 62, 68
//        End If
//        'Print the level's name
//        If WorldPlayer(1).LevelName <> "" Then
//            SuperPrint WorldPlayer(1).LevelName, 2, 32 + (48 * A) + 116, 109
//        End If
//        If GamePaused = True Then
//            BitBlt myBackBuffer, 210, 200, 380, 200, 0, 0, 0, vbWhiteness
//            If Cheater = False Then
//                SuperPrint "CONTINUE", 3, 272, 257
//                SuperPrint "SAVE & CONTINUE", 3, 272, 292
//                SuperPrint "SAVE & QUIT", 3, 272, 327
//                BitBlt myBackBuffer, 252, 257 + (MenuCursor * 35), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, 252, 257 + (MenuCursor * 35), 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
//            Else
//                SuperPrint "CONTINUE", 3, 272 + 56, 275
//                SuperPrint "QUIT", 3, 272 + 56, 310
//                BitBlt myBackBuffer, 252 + 56, 275 + (MenuCursor * 35), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, 252 + 56, 275 + (MenuCursor * 35), 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
//            End If
//        End If
//        If PrintFPS > 0 Then
//            SuperPrint Str(PrintFPS), 1, 8, 8
//        End If
//        'BitBlt frmMain.hdc, 0, 0, ScreenW, ScreenH, myBackBuffer, 0, 0, vbSrcCopy
//        StretchBlt frmMain.hdc, 0, 0, frmMain.ScaleWidth, frmMain.ScaleHeight, myBackBuffer, 0, 0, 800, 600, vbSrcCopy
//    End If
//    If TakeScreen = True Then ScreenShot
    if(TakeScreen)
        ScreenShot();
}

// This draws the graphic to the screen when in a level/game menu/outro/level editor
void UpdateGraphics()
{
//    On Error Resume Next

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
//        If GetTickCount + Int(1000 * (1 - (cycleCount / 63))) > GoalTime Then   'Don't draw this frame
//            numScreens = 1
//            If LevelEditor = False Then
//                If ScreenType = 1 Then numScreens = 2
//                If ScreenType = 4 Then numScreens = 2
//                If ScreenType = 5 Then
//                    DynamicScreen
//                    If vScreen(2).Visible = True Then
//                        numScreens = 2
//                    Else
//                        numScreens = 1
//                    End If
//                End If
//                If ScreenType = 8 Then numScreens = 1
//            End If
//            For Z = 1 To numScreens
//                If LevelEditor = False Then
//                    If ScreenType = 2 Or ScreenType = 3 Then
//                        GetvScreenAverage
//                    ElseIf ScreenType = 5 And vScreen(2).Visible = False Then
//                        GetvScreenAverage
//                    ElseIf ScreenType = 7 Then
//                        GetvScreenCredits
//                    Else
//                        GetvScreen Z
//                    End If
//                End If
//                For A = 1 To numNPCs
//                    With NPC(A)
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            If .Reset(Z) = True Or .Active = True Then
//                                If .Active = False Then
//                                    .JustActivated = Z
//                                    If nPlay.Online = True Then
//                                        Netplay.sendData "2a" & A & "|" & nPlay.MySlot + 1 & LB
//                                        .JustActivated = nPlay.MySlot + 1
//                                    End If
//                                End If
//                                .TimeLeft = Physics.NPCTimeOffScreen
//                                If nPlay.Online = True And nPlay.NPCWaitCount >= 10 And nPlay.Mode = 0 Then timeStr = timeStr & "2b" & A & LB
//                                .Active = True
//                            End If
//                            .Reset(1) = False
//                            .Reset(2) = False
//                        Else
//                            .Reset(Z) = True
//                            If numScreens = 1 Then .Reset(2) = True
//                            If SingleCoop = 1 Then
//                                .Reset(2) = True
//                            ElseIf SingleCoop = 2 Then
//                                .Reset(1) = True
//                            End If
//                        End If
//                    End With
//                Next A
//            Next Z
//            Exit Sub
//        End If
//    End If
    }
//    fpsCount = fpsCount + 1
    fpsCount += 1;

//    Dim SuperText As String
    std::string SuperText;
//    Dim tempText As String
    std::string tempText;
//    Dim BoxY As Integer
    int BoxY = 0;
//    Dim tempBool As Boolean
    bool tempBool = false;
//    Dim B As Integer
    int B = 0;
//    Dim B2 As Integer
    int B2 = 0;
//    Dim C As Integer
    int C = 0;
//    Dim D As Integer
    int D = 0;
//    Dim E As Integer
    int E = 0;
//    Dim d2 As Integer
    int d2 = 0;
//    Dim e2 As Integer
    int e2 = 0;
//    Dim X As Integer
    int X = 0;
//    Dim Y As Integer
    int Y = 0;
//    Dim fBlock As Integer
    int fBlock = 0;
//    Dim lBlock As Integer
    int lBlock = 0;
//    Dim tempLocation As Location
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
//        BackgroundFrameCount[26] = BackgroundFrameCount[26] + 1
        BackgroundFrameCount[26] += 1;
//        If BackgroundFrameCount[26] >= 8 Then
        if(BackgroundFrameCount[26] >= 8) {
//            BackgroundFrame[26] = BackgroundFrame[26] + 1
            BackgroundFrame[26] += 1;
//            If BackgroundFrame[26] >= 8 Then BackgroundFrame[26] = 0
            if(BackgroundFrame[26] >= 8)
                BackgroundFrame[26] = 0;
//            BackgroundFrameCount[26] = 0
            BackgroundFrameCount[26] = 0;
//        End If
        }

//        BackgroundFrameCount[18] = BackgroundFrameCount[18] + 1
        BackgroundFrameCount[18] += 1;
//        If BackgroundFrameCount[18] >= 12 Then
        if(BackgroundFrameCount[18] >= 12)
        {
//            BackgroundFrame[18] = BackgroundFrame[18] + 1
            BackgroundFrame[18] += 1;
//            If BackgroundFrame[18] >= 4 Then BackgroundFrame[18] = 0
            if(BackgroundFrame[18] >= 4)
                BackgroundFrame[18] = 0;
//            BackgroundFrame[19] = BackgroundFrame[18]
            BackgroundFrame[19] = BackgroundFrame[18];
//            BackgroundFrame[20] = BackgroundFrame[18]
            BackgroundFrame[20] = BackgroundFrame[18];
//            BackgroundFrame[161] = BackgroundFrame[18]
            BackgroundFrame[161] = BackgroundFrame[18];
//            BackgroundFrameCount[18] = 0
            BackgroundFrameCount[18] = 0;
//        End If
        }

//        BackgroundFrameCount[36] = BackgroundFrameCount[36] + 1
//        If BackgroundFrameCount[36] >= 2 Then
//            BackgroundFrame[36] = BackgroundFrame[36] + 1
//            If BackgroundFrame[36] >= 4 Then BackgroundFrame[36] = 0
//            BackgroundFrameCount[36] = 0
//        End If
//        BackgroundFrame[68] = BackgroundFrame[36]
//        BackgroundFrameCount[65] = BackgroundFrameCount[65] + 1
//        If BackgroundFrameCount[65] >= 8 Then
//            BackgroundFrame[65] = BackgroundFrame[65] + 1
//            If BackgroundFrame[65] >= 4 Then BackgroundFrame[65] = 0
//            BackgroundFrameCount[65] = 0
//        End If

//        BackgroundFrame[66] = BackgroundFrame[65]

//        BackgroundFrame[70] = BackgroundFrame[65]
//        BackgroundFrame[100] = BackgroundFrame[65]

//        BackgroundFrame[134] = BackgroundFrame[65]
//        BackgroundFrame[135] = BackgroundFrame[65]
//        BackgroundFrame[136] = BackgroundFrame[65]
//        BackgroundFrame[137] = BackgroundFrame[65]
//        BackgroundFrame[138] = BackgroundFrame[65]


//        BackgroundFrameCount[82] = BackgroundFrameCount[82] + 1
//        If BackgroundFrameCount[82] >= 10 Then
//            BackgroundFrame[82] = BackgroundFrame[82] + 1
//            If BackgroundFrame[82] >= 4 Then BackgroundFrame[82] = 0
//            BackgroundFrameCount[82] = 0
//        End If

//        BackgroundFrameCount[170] = BackgroundFrameCount[170] + 1
//        If BackgroundFrameCount[170] >= 8 Then
//            BackgroundFrame[170] = BackgroundFrame[170] + 1
//            If BackgroundFrame[170] >= 4 Then BackgroundFrame[170] = 0
//            BackgroundFrame[171] = BackgroundFrame[170]
//            BackgroundFrameCount[170] = 0
//        End If

//        BackgroundFrameCount[125] = BackgroundFrameCount[125] + 1
//        If BackgroundFrameCount[125] >= 4 Then
//            If BackgroundFrame[125] = 0 Then
//                BackgroundFrame[125] = 1
//            Else
//                BackgroundFrame[125] = 0
//            End If
//            BackgroundFrameCount[125] = 0
//        End If
//        SpecialFrames
//    End If
    }

//        BackgroundFrame(172) = BackgroundFrame(66)
    BackgroundFrame[172] = BackgroundFrame[66];
//    BackgroundFrameCount(158) = BackgroundFrameCount(158) + 1
    BackgroundFrameCount[158] += 1;
//    If BackgroundFrameCount(158) >= 6 Then
    if(BackgroundFrameCount[158] >= 6)
    {
//        BackgroundFrameCount(158) = 0
//        BackgroundFrame(158) = BackgroundFrame(158) + 1
//        BackgroundFrame(159) = BackgroundFrame(159) + 1
//        If BackgroundFrame(158) >= 4 Then BackgroundFrame(158) = 0
//        If BackgroundFrame(159) >= 8 Then BackgroundFrame(159) = 0
//    End If
    }


//    BackgroundFrameCount(168) = BackgroundFrameCount(168) + 1
//    If BackgroundFrameCount(168) >= 8 Then
//        BackgroundFrame(168) = BackgroundFrame(168) + 1
//        If BackgroundFrame(168) >= 8 Then BackgroundFrame(168) = 0
//        BackgroundFrameCount(168) = 0
//    End If

//    BackgroundFrameCount(173) = BackgroundFrameCount(173) + 1
//    If BackgroundFrameCount(173) >= 8 Then
//        BackgroundFrameCount(173) = 0
//        If BackgroundFrame(173) = 0 Then
//            BackgroundFrame(173) = 1
//        Else
//            BackgroundFrame(173) = 0
//        End If
//    End If

//    BackgroundFrameCount(187) = BackgroundFrameCount(187) + 1
//    If BackgroundFrameCount(187) >= 6 Then
//        BackgroundFrame(187) = BackgroundFrame(187) + 1
//        If BackgroundFrame(187) >= 4 Then BackgroundFrame(187) = 0
//        BackgroundFrame(188) = BackgroundFrame(187)
//        BackgroundFrame(189) = BackgroundFrame(187)
//        BackgroundFrame(190) = BackgroundFrame(187)
//        BackgroundFrameCount(187) = 0
//    End If


//    If LevelEditor = False Then  'Sets up the screens if not in level editor
    { // NOT AN EDITOR
//        If ScreenType = 1 Then numScreens = 2
        if(ScreenType == 1)
            numScreens = 2;
//        If ScreenType = 4 Then numScreens = 2
        if(ScreenType == 4)
            numScreens = 2;
//        If ScreenType = 5 Then
        if(ScreenType == 5)
        {
//            DynamicScreen
            DynamicScreen();
//            If vScreen(2).Visible = True Then
            if(vScreen[2].Visible)
//                numScreens = 2
                numScreens = 2;
//            Else
            else
//                numScreens = 1
                numScreens = 1;
//            End If
//        End If
        }
//        If ScreenType = 8 Then numScreens = 1
        if(ScreenType == 8)
            numScreens = 1;
//    End If
    }

//    If ClearBuffer = True Then
    if(ClearBuffer)
    {
//        ClearBuffer = False
        ClearBuffer = False;
//        BitBlt myBackBuffer, 0, 0, ScreenW, ScreenH, 0, 0, 0, vbWhiteness
//        BitBlt frmMain.hdc, 0, 0, frmMain.ScaleWidth, frmMain.ScaleHeight, 0, 0, 0, vbWhiteness
        frmMain.clearBuffer();
//    End If
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

//        If qScreen = True Then
//            If vScreenX(1) < qScreenX(1) - 2 Then
//                qScreenX(1) = qScreenX(1) - 2
//            ElseIf vScreenX(1) > qScreenX(1) + 2 Then
//                qScreenX(1) = qScreenX(1) + 2
//            End If
//            If vScreenY(1) < qScreenY(1) - 2 Then
//                qScreenY(1) = qScreenY(1) - 2
//            ElseIf vScreenY(1) > qScreenY(1) + 2 Then
//                qScreenY(1) = qScreenY(1) + 2
//            End If
//            If qScreenX(1) < vScreenX(1) + 5 And qScreenX(1) > vScreenX(1) - 5 And qScreenY(1) < vScreenY(1) + 5 And qScreenY(1) > vScreenY(1) - 5 Then
//                qScreen = False
//            End If
//            vScreenX(1) = qScreenX(1)
//            vScreenY(1) = qScreenY(1)
//        End If

//        'noturningback
//        If LevelEditor = False Then
//            If NoTurnBack(Player(Z).Section) = True Then
//                A = Z
//                If numScreens > 1 Then
//                    If Player(1).Section = Player(2).Section Then
//                        If Z = 1 Then GetvScreen 2
//                        If -vScreenX(1) < -vScreenX(2) Then
//                            A = 1
//                        Else
//                            A = 2
//                        End If
//                    End If
//                End If
//                If -vScreenX(A) > level(S).X Then
//                    LevelChop(S) = LevelChop(S) + -vScreenX(A) - level(S).X
//                    level(S).X = -vScreenX(A)
//                End If
//            End If
//        End If
//        DrawBackground S, Z


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
            // Curtain
            frmMain.renderTexture(0, 0, GFX.MenuGFX[1]);
            // Game logo
            frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 70, GFX.MenuGFX[2]);
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
//                            For B = 1 To numPlayers
//                                If Player(B).Dead = False And Player(B).TimeToLive = 0 And Player(B).Section = .Section And vScreenCollision(Z, Player(B).Location) = True Then
//                                    If C = 0 Or Abs(.Location.X + .Location.Width / 2 - (Player(B).Location.X + Player(B).Location.Width / 2)) < C Then
//                                        C = Abs(.Location.X + .Location.Width / 2 - (Player(B).Location.X + Player(B).Location.Width / 2))
//                                        D = B
//                                    End If
//                                End If
//                            Next B
//                            If C = 0 Then
//                                For B = 1 To numPlayers
//                                    If Player(B).Dead = False And Player(B).TimeToLive = 0 And Player(B).Section = .Section Then
//                                        If C = 0 Or Abs(.Location.X + .Location.Width / 2 - (Player(B).Location.X + Player(B).Location.Width / 2)) < C Then
//                                            C = Abs(.Location.X + .Location.Width / 2 - (Player(B).Location.X + Player(B).Location.Width / 2))
//                                            D = B
//                                        End If
//                                    End If
//                                Next B
//                            End If
//                            .Location.X = Player(D).Location.X + Player(D).Location.Width / 2 - .Location.Width / 2
//                            .Location.Y = Player(D).Location.Y + Player(D).Location.Height - .Location.Height
//                            .Section = Player(D).Section
//                            .Location.SpeedX = Player(D).Location.SpeedX
//                            .Location.SpeedY = Player(D).Location.SpeedY
//                            .Location.SpeedY = Rnd * 12 - 6
//                            .CanJump = True
//                        End If
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
            For(A, 1, MidBackground - 1)
            {
//                With Background(A)
//                    If BackgroundHasNoMask(Background(A).Type) = False Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcPaint
//                        End If
//                    Else
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, GFXBackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcCopy
//                        End If
//                    End If
//                End With
//            Next A
            }
//        End If
        }

        tempLocation.Width = 32;
        tempLocation.Height = 32;
//        For A = 1 To sBlockNum 'Display sizable blocks
        For(A, 1, sBlockNum) // Display sizable blocks
        {
//            With Block(sBlockArray(A))
//                If BlockIsSizable(.Type) And (Not .Invis = True Or LevelEditor = True) Then
//                    If vScreenCollision(Z, .Location) And .Hidden = False Then
//                        For B = 0 To (.Location.Height / 32) - 1
//                            For C = 0 To (.Location.Width / 32) - 1
//                                tempLocation.X = .Location.X + C * 32
//                                tempLocation.Y = .Location.Y + B * 32
//                                If vScreenCollision(Z, tempLocation) Then
//                                    D = C
//                                    E = B
//                                    If Not D = 0 Then
//                                        If D = (.Location.Width / 32) - 1 Then
//                                            D = 2
//                                        Else
//                                            D = 1
//                                            d2 = 0.5
//                                        End If
//                                    End If
//                                    If Not E = 0 Then
//                                        If E = (.Location.Height / 32) - 1 Then
//                                            E = 2
//                                        Else
//                                            E = 1
//                                        End If
//                                    End If
//                                    If (D = 0 Or D = 2) Or (E = 0 Or E = 2) Or (.Type = 130 And (D = 0 Or D = 2) And E = 1) Then
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + C * 32, vScreenY(Z) + .Location.Y + B * 32, 32, 32, GFXBlockMask(.Type), D * 32, E * 32, vbSrcAnd
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + C * 32, vScreenY(Z) + .Location.Y + B * 32, 32, 32, GFXBlock(.Type), D * 32, E * 32, vbSrcPaint
//                                    Else
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + C * 32, vScreenY(Z) + .Location.Y + B * 32, 32, 32, GFXBlock(.Type), D * 32, E * 32, vbSrcCopy
//                                    End If
//                                End If
//                            Next C
//                        Next B
//                    End If
//                End If
//            End With
//        Next A
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
//            For A = MidBackground To LastBackground 'Second backgrounds
//                With Background(A)
//                    If vScreenCollision(Z, .Location) And .Hidden = False Then
//                        If BackgroundHasNoMask(.Type) = False Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcPaint
//                        Else
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcCopy
//                        End If
//                    End If
//                End With
//            Next A
//        End If
        }

//        For A = numBackground + 1 To numBackground + numLocked 'Locked doors
        For(A, numBackground + 1, numBackground + numLocked)
        {
//            With Background(A)
//                If vScreenCollision(Z, .Location) And (.Type = 98 Or .Type = 160) And .Hidden = False Then
//                    If BackgroundHasNoMask(.Type) = False Then
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcAnd
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcCopy
//                    End If
//                End If
//            End With
//        Next A
        }

//        For A = 1 To numNPCs 'Display NPCs that should be behind blocks
        For(A, 1, numNPCs) // Display NPCs that should be behind blocks
        {
//            With NPC(A)
//                If (.Effect = 208 Or NPCIsAVine(.Type) = True Or .Type = 209 Or .Type = 159 Or .Type = 245 Or .Type = 8 Or .Type = 93 Or .Type = 74 Or .Type = 256 Or .Type = 257 Or .Type = 51 Or .Type = 52 Or .Effect = 1 Or .Effect = 3 Or .Effect = 4 Or (.Type = 45 And .Special = 0)) And .standingOnPlayer = 0 And (.Generator = False Or LevelEditor = True) Or .Type = 179 Or .Type = 270 Then
//                    If Not .Effect = 2 And (.Generator = False Or LevelEditor = True) Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            If .Active = True Then
//                                If .Type = 8 Or .Type = 74 Or .Type = 93 Or .Type = 245 Or .Type = 256 Or .Type = 270 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * NPCHeight(.Type), vbSrcAnd
//                                    If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * NPCHeight(.Type), vbSrcPaint
//                                ElseIf .Type = 51 Or .Type = 257 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * NPCHeight(.Type) + NPCHeight(.Type) - .Location.Height, vbSrcAnd
//                                    If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * NPCHeight(.Type) + NPCHeight(.Type) - .Location.Height, vbSrcPaint
//                                ElseIf .Type = 52 Then
//                                    If .Direction = -1 Then
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * NPCHeight(.Type), vbSrcAnd
//                                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * NPCHeight(.Type), vbSrcPaint
//                                    Else
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), NPCWidth(.Type) - .Location.Width, .Frame * NPCHeight(.Type), vbSrcAnd
//                                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), NPCWidth(.Type) - .Location.Width, .Frame * NPCHeight(.Type), vbSrcPaint
//                                    End If
//                                ElseIf NPCWidthGFX(.Type) = 0 Or .Effect = 1 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * NPCHeight(.Type), vbSrcAnd
//                                    If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * NPCHeight(.Type), vbSrcPaint
//                                Else
//                                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPCMask(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcAnd
//                                    If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPC(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcPaint
//                                End If
//                            End If
//                            If .Reset(Z) = True Or .Active = True Then
//                                If .Active = False Then
//                                    .JustActivated = Z
//                                    If nPlay.Online = True Then
//                                        Netplay.sendData "2a" & A & "|" & nPlay.MySlot + 1 & LB
//                                        .JustActivated = nPlay.MySlot + 1
//                                    End If
//                                End If
//                                .TimeLeft = Physics.NPCTimeOffScreen
//                                If nPlay.Online = True And nPlay.NPCWaitCount >= 10 And nPlay.Mode = 0 Then timeStr = timeStr & "2b" & A & LB
//                                .Active = True
//                            End If
//                            .Reset(1) = False
//                            .Reset(2) = False
//                        Else
//                            .Reset(Z) = True
//                            If numScreens = 1 Then .Reset(2) = True
//                            If SingleCoop = 1 Then
//                                .Reset(2) = True
//                            ElseIf SingleCoop = 2 Then
//                                .Reset(1) = True
//                            End If
//                        End If
//                    End If
//                End If
//            End With
//        Next A
        }


//        For A = 1 To numPlayers 'Players behind blocks
        For(A, 1, numPlayers)
        {
//            With Player(A)
//                If .Dead = False And .Immune2 = False And .TimeToLive = 0 And .Effect = 3 Then
//                    If vScreenCollision(Z, .Location) Then
//                        Dim Y2 As Single
//                        Dim X2 As Single
//                        'warp NPCs
//                        If .HoldingNPC > 0 And .Frame <> 15 Then
//                            With NPC(.HoldingNPC)
//                                If (vScreenCollision(Z, .Location) Or vScreenCollision(Z, newLoc(.Location.X - (NPCWidthGFX(.Type) - .Location.Width) / 2, .Location.Y, CDbl(NPCWidthGFX(.Type)), CDbl(NPCHeight(.Type))))) And .Hidden = False Then
//                                    tempLocation = .Location
//                                    If NPCHeightGFX(.Type) <> 0 Or NPCWidthGFX(.Type) <> 0 Then
//                                        tempLocation.Height = NPCHeightGFX(.Type)
//                                        tempLocation.Width = NPCWidthGFX(.Type)
//                                        tempLocation.Y = .Location.Y + .Location.Height - NPCHeightGFX(.Type)
//                                        tempLocation.X = .Location.X + .Location.Width / 2 - NPCWidthGFX(.Type) / 2
//                                    Else
//                                        tempLocation.Height = NPCHeight(.Type)
//                                        tempLocation.Width = NPCWidth(.Type)
//                                    End If
//                                    tempLocation.X = tempLocation.X + NPCFrameOffsetX(.Type)
//                                    tempLocation.Y = tempLocation.Y + NPCFrameOffsetY(.Type)
//                                    Y2 = 0
//                                    X2 = 0
//                                    NPCWarpGFX A, tempLocation, X2, Y2
//                                    If NPCHeightGFX(.Type) <> 0 Or NPCWidthGFX(.Type) <> 0 Then
//                                        BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(.Type), X2, Y2 + .Frame * NPCHeightGFX(.Type), vbSrcAnd
//                                        BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(.Type), X2, Y2 + .Frame * NPCHeightGFX(.Type), vbSrcPaint
//                                    Else
//                                        BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(.Type), X2, Y2 + .Frame * NPCHeight(.Type), vbSrcAnd
//                                        BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(.Type), X2, Y2 + .Frame * NPCHeight(.Type), vbSrcPaint
//                                    End If

//                                End If
//                            End With
//                        End If


//                        If .Mount = 3 Then
//                            B = .MountType
//                            'Yoshi's Body
//                            tempLocation = .Location
//                            tempLocation.Height = 32
//                            tempLocation.Width = 32
//                            tempLocation.X = tempLocation.X + .YoshiBX
//                            tempLocation.Y = tempLocation.Y + .YoshiBY
//                            Y2 = 0
//                            X2 = 0
//                            PlayerWarpGFX A, tempLocation, X2, Y2
//                            BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXYoshiBMask(B), X2, Y2 + 32 * .YoshiBFrame, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXYoshiB(B), X2, Y2 + 32 * .YoshiBFrame, vbSrcPaint
//                            'Yoshi's Head
//                            tempLocation = .Location
//                            tempLocation.Height = 32
//                            tempLocation.Width = 32
//                            tempLocation.X = tempLocation.X + .YoshiTX
//                            tempLocation.Y = tempLocation.Y + .YoshiTY
//                            Y2 = 0
//                            X2 = 0
//                            PlayerWarpGFX A, tempLocation, X2, Y2
//                            BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXYoshiTMask(B), X2, Y2 + 32 * .YoshiTFrame, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXYoshiT(B), X2, Y2 + 32 * .YoshiTFrame, vbSrcPaint
//                        End If
//                        If .Character = 1 Then
//                            If .Mount = 1 Then
//                                tempLocation = .Location
//                                If .State = 1 Then
//                                    tempLocation.Height = .Location.Height - MarioFrameY((.State * 100) + (.Frame * .Direction))
//                                Else
//                                    tempLocation.Height = .Location.Height - MarioFrameY((.State * 100) + (.Frame * .Direction)) - 30
//                                End If
//                                tempLocation.Width = 99
//                                tempLocation.X = tempLocation.X + MarioFrameX((.State * 100) + (.Frame * .Direction))
//                                tempLocation.Y = tempLocation.Y + MarioFrameY((.State * 100) + (.Frame * .Direction))
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXMarioMask(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcAnd
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXMario(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcPaint
//                                tempLocation = .Location
//                                tempLocation.Height = 32
//                                tempLocation.Width = 32
//                                tempLocation.X = tempLocation.X + .Location.Width / 2 - 16
//                                tempLocation.Y = tempLocation.Y + .Location.Height - 30
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.BootMask(.MountType).hdc, X2, Y2 + 32 * .MountFrame, vbSrcAnd
//                                If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot(.MountType).hdc, X2, Y2 + 32 * .MountFrame, vbSrcPaint
//                            Else
//                                tempLocation = .Location
//                                tempLocation.Height = 99
//                                tempLocation.Width = 99
//                                tempLocation.X = tempLocation.X + MarioFrameX((.State * 100) + (.Frame * .Direction))
//                                tempLocation.Y = tempLocation.Y + MarioFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXMarioMask(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcAnd
//                                If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXMario(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcPaint
//                            End If
//                        ElseIf .Character = 2 Then
//                           If .Mount = 1 Then
//                                tempLocation = .Location
//                                If .State = 1 Then
//                                    tempLocation.Height = .Location.Height - LuigiFrameY((.State * 100) + (.Frame * .Direction))
//                                Else
//                                    tempLocation.Height = .Location.Height - LuigiFrameY((.State * 100) + (.Frame * .Direction)) - 30
//                                End If
//                                tempLocation.Width = 99
//                                tempLocation.X = tempLocation.X + LuigiFrameX((.State * 100) + (.Frame * .Direction))
//                                tempLocation.Y = tempLocation.Y + LuigiFrameY((.State * 100) + (.Frame * .Direction))
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLuigiMask(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcAnd
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLuigi(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcPaint
//                                tempLocation = .Location
//                                tempLocation.Height = 32
//                                tempLocation.Width = 32
//                                tempLocation.X = tempLocation.X + .Location.Width / 2 - 16
//                                tempLocation.Y = tempLocation.Y + .Location.Height - 30
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.BootMask(.MountType).hdc, X2, Y2 + 32 * .MountFrame, vbSrcAnd
//                                If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot(.MountType).hdc, X2, Y2 + 32 * .MountFrame, vbSrcPaint
//                            Else
//                                tempLocation = .Location
//                                tempLocation.Height = 99
//                                tempLocation.Width = 99
//                                tempLocation.X = tempLocation.X + LuigiFrameX((.State * 100) + (.Frame * .Direction))
//                                tempLocation.Y = tempLocation.Y + LuigiFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLuigiMask(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcAnd
//                                If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLuigi(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcPaint
//                            End If
//                        ElseIf .Character = 3 Then
//                           If .Mount = 1 Then
//                                tempLocation = .Location
//                                If .State = 1 Then
//                                    tempLocation.Height = .Location.Height - PeachFrameY((.State * 100) + (.Frame * .Direction))
//                                Else
//                                    tempLocation.Height = .Location.Height - PeachFrameY((.State * 100) + (.Frame * .Direction)) - 30
//                                End If
//                                tempLocation.Width = 99
//                                tempLocation.X = tempLocation.X + PeachFrameX((.State * 100) + (.Frame * .Direction))
//                                tempLocation.Y = tempLocation.Y + PeachFrameY((.State * 100) + (.Frame * .Direction))
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXPeachMask(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcAnd
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXPeach(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcPaint
//                                tempLocation = .Location
//                                tempLocation.Height = 32
//                                tempLocation.Width = 32
//                                tempLocation.X = tempLocation.X + .Location.Width / 2 - 16
//                                tempLocation.Y = tempLocation.Y + .Location.Height - 30
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.BootMask(.MountType).hdc, X2, Y2 + 32 * .MountFrame, vbSrcAnd
//                                If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot(.MountType).hdc, X2, Y2 + 32 * .MountFrame, vbSrcPaint
//                            Else
//                                tempLocation = .Location
//                                tempLocation.Height = 99
//                                tempLocation.Width = 99
//                                tempLocation.X = tempLocation.X + PeachFrameX((.State * 100) + (.Frame * .Direction))
//                                tempLocation.Y = tempLocation.Y + PeachFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXPeachMask(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcAnd
//                                If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXPeach(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcPaint
//                            End If
//                        ElseIf .Character = 4 Then
//                           If .Mount = 1 Then
//                                tempLocation = .Location
//                                If .State = 1 Then
//                                    tempLocation.Height = .Location.Height - ToadFrameY((.State * 100) + (.Frame * .Direction))
//                                Else
//                                    tempLocation.Height = .Location.Height - ToadFrameY((.State * 100) + (.Frame * .Direction)) - 26
//                                End If
//                                tempLocation.Width = 99
//                                tempLocation.X = tempLocation.X + ToadFrameX((.State * 100) + (.Frame * .Direction))
//                                If .State = 1 Then
//                                    tempLocation.Y = tempLocation.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)) + 6
//                                Else
//                                    tempLocation.Y = tempLocation.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)) - 4
//                                End If
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXToadMask(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcAnd
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXToad(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcPaint
//                                tempLocation = .Location
//                                tempLocation.Height = 32
//                                tempLocation.Width = 32
//                                tempLocation.X = tempLocation.X + .Location.Width / 2 - 16
//                                tempLocation.Y = tempLocation.Y + .Location.Height - 30
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.BootMask(.MountType).hdc, X2, Y2 + 32 * .MountFrame, vbSrcAnd
//                                If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFX.Boot(.MountType).hdc, X2, Y2 + 32 * .MountFrame, vbSrcPaint
//                            Else
//                                tempLocation = .Location
//                                tempLocation.Height = 99
//                                tempLocation.Width = 99
//                                tempLocation.X = tempLocation.X + ToadFrameX((.State * 100) + (.Frame * .Direction))
//                                tempLocation.Y = tempLocation.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY
//                                Y2 = 0
//                                X2 = 0
//                                PlayerWarpGFX A, tempLocation, X2, Y2
//                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXToadMask(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcAnd
//                                If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXToad(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcPaint
//                            End If
//                        ElseIf .Character = 5 Then
//                            If .Frame > 5 Then .Frame = 1
//                            tempLocation = .Location
//                            tempLocation.Height = 99
//                            tempLocation.Width = 99
//                            tempLocation.X = tempLocation.X + LinkFrameX((.State * 100) + (.Frame * .Direction))
//                            tempLocation.Y = tempLocation.Y + LinkFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY
//                            Y2 = 0
//                            X2 = 0
//                            PlayerWarpGFX A, tempLocation, X2, Y2
//                            BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLinkMask(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcAnd
//                            If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXLink(.State), pfrX(100 + .Frame * .Direction) + X2, pfrY(100 + .Frame * .Direction) + Y2, vbSrcPaint
//                        End If
//                    End If
//                    If .HoldingNPC > 0 And .Frame = 15 Then
//                        With NPC(.HoldingNPC)
//                            If (vScreenCollision(Z, .Location) Or vScreenCollision(Z, newLoc(.Location.X - (NPCWidthGFX(.Type) - .Location.Width) / 2, .Location.Y, CDbl(NPCWidthGFX(.Type)), CDbl(NPCHeight(.Type))))) And .Hidden = False And .Type <> 263 Then
//                                tempLocation = .Location
//                                If NPCHeightGFX(.Type) <> 0 Or NPCWidthGFX(.Type) <> 0 Then
//                                    tempLocation.Height = NPCHeightGFX(.Type)
//                                    tempLocation.Width = NPCWidthGFX(.Type)
//                                    tempLocation.Y = .Location.Y + .Location.Height - NPCHeightGFX(.Type)
//                                    tempLocation.X = .Location.X + .Location.Width / 2 - NPCWidthGFX(.Type) / 2
//                                Else
//                                    tempLocation.Height = NPCHeight(.Type)
//                                    tempLocation.Width = NPCWidth(.Type)
//                                End If
//                                tempLocation.X = tempLocation.X + NPCFrameOffsetX(.Type)
//                                tempLocation.Y = tempLocation.Y + NPCFrameOffsetY(.Type)
//                                Y2 = 0
//                                X2 = 0
//                                NPCWarpGFX A, tempLocation, X2, Y2
//                                If NPCHeightGFX(.Type) <> 0 Or NPCWidthGFX(.Type) <> 0 Then
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(.Type), X2, Y2 + .Frame * NPCHeightGFX(.Type), vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(.Type), X2, Y2 + .Frame * NPCHeightGFX(.Type), vbSrcPaint
//                                Else
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(.Type), X2, Y2 + .Frame * NPCHeight(.Type), vbSrcAnd
//                                    BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X, vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(.Type), X2, Y2 + .Frame * NPCHeight(.Type), vbSrcPaint
//                                End If

//                            End If
//                        End With
//                    End If
//                End If
//            End With
//        Next A
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
//            With Block(A)
//                If Not BlockIsSizable(.Type) And (Not .Invis = True Or (LevelEditor = True And BlockFlash <= 30)) And Not .Type = 0 And Not BlockKills(.Type) Then
//                    If vScreenCollision(Z, .Location) And .Hidden = False Then
//                        If BlockHasNoMask(.Type) = True Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .ShakeY3, .Location.Width, .Location.Height, GFXBlock(.Type), 0, BlockFrame(.Type) * 32, vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .ShakeY3, .Location.Width, .Location.Height, GFXBlockMask(.Type), 0, BlockFrame(.Type) * 32, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .ShakeY3, .Location.Width, .Location.Height, GFXBlock(.Type), 0, BlockFrame(.Type) * 32, vbSrcPaint
//                        End If
//                    End If
//                End If
//            End With
//        Next A
        }

//'effects in back
//        For A = 1 To numEffects
//            With Effect(A)
//                If .Type = 112 Or .Type = 54 Or .Type = 55 Or .Type = 59 Or .Type = 77 Or .Type = 81 Or .Type = 82 Or .Type = 103 Or .Type = 104 Or .Type = 114 Or .Type = 123 Or .Type = 124 Then
//                    If vScreenCollision(Z, .Location) Then
//                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffectMask(.Type), 0, .Frame * EffectHeight(.Type), vbSrcAnd
//                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, .Location.Width, .Location.Height, GFXEffect(.Type), 0, .Frame * EffectHeight(.Type), vbSrcPaint
//                    End If
//                End If
//            End With
//        Next A


//        For A = 1 To numNPCs 'Display NPCs that should be behind other npcs
//            With NPC(A)
//                If .Effect = 0 Then
//                    If .HoldingPlayer = 0 And (.standingOnPlayer > 0 Or .Type = 56 Or .Type = 22 Or .Type = 49 Or .Type = 91 Or .Type = 160 Or .Type = 282 Or NPCIsACoin(.Type)) And (.Generator = False Or LevelEditor = True) Then
//                        'If Not NPCIsACoin(.Type) Then
//                            If (vScreenCollision(Z, .Location) Or vScreenCollision(Z, newLoc(.Location.X - (NPCWidthGFX(.Type) - .Location.Width) / 2, .Location.Y, CDbl(NPCWidthGFX(.Type)), CDbl(NPCHeight(.Type))))) And .Hidden = False Then
//                                If .Active = True Then
//                                    If NPCWidthGFX(.Type) = 0 Then
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * .Location.Height, vbSrcAnd
//                                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * .Location.Height, vbSrcPaint
//                                    Else
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + (NPCFrameOffsetX(.Type) * -.Direction) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPCMask(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcAnd
//                                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + (NPCFrameOffsetX(.Type) * -.Direction) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPC(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcPaint
//                                    End If
//                                End If
//                                If .Reset(Z) = True Or .Active = True Then
//                                    If .Active = False Then
//                                        .JustActivated = Z
//                                        If nPlay.Online = True Then
//                                            Netplay.sendData "2a" & A & "|" & nPlay.MySlot + 1 & LB
//                                            .JustActivated = nPlay.MySlot + 1
//                                        End If
//                                    End If
//                                    .TimeLeft = Physics.NPCTimeOffScreen
//                                    If nPlay.Online = True And nPlay.NPCWaitCount >= 10 And nPlay.Mode = 0 Then timeStr = timeStr & "2b" & A & LB
//                                    .Active = True
//                                 End If
//                                .Reset(1) = False
//                                .Reset(2) = False
//                            Else
//                                .Reset(Z) = True
//                                If numScreens = 1 Then .Reset(2) = True
//                                If SingleCoop = 1 Then
//                                    .Reset(2) = True
//                                ElseIf SingleCoop = 2 Then
//                                    .Reset(1) = True
//                                End If
//                            End If
//                        'End If
//                    End If
//                End If
//            End With
//        Next A


//        For A = 1 To numNPCs 'ice
//            With NPC(A)
//                If .Type = 263 And .Effect = 0 And .HoldingPlayer = 0 Then
//                    If (vScreenCollision(Z, .Location) Or vScreenCollision(Z, newLoc(.Location.X - (NPCWidthGFX(.Type) - .Location.Width) / 2, .Location.Y, CDbl(NPCWidthGFX(.Type)), CDbl(NPCHeight(.Type))))) And .Hidden = False Then
//                        DrawFrozenNPC Z, A
//                        If .Reset(Z) = True Or .Active = True Then
//                            If .Active = False Then
//                                .JustActivated = Z
//                                If nPlay.Online = True Then
//                                    Netplay.sendData "2a" & A & "|" & nPlay.MySlot + 1 & LB
//                                    .JustActivated = nPlay.MySlot + 1
//                                End If
//                            End If
//                            .TimeLeft = Physics.NPCTimeOffScreen
//                            If nPlay.Online = True And nPlay.NPCWaitCount >= 10 And nPlay.Mode = 0 Then timeStr = timeStr & "2b" & A & LB
//                            .Active = True
//                         End If
//                        .Reset(1) = False
//                        .Reset(2) = False
//                    Else
//                        .Reset(Z) = True
//                        If numScreens = 1 Then .Reset(2) = True
//                        If SingleCoop = 1 Then
//                            .Reset(2) = True
//                        ElseIf SingleCoop = 2 Then
//                            .Reset(1) = True
//                        End If
//                    End If
//                End If
//            End With
//        Next A


//        For A = 1 To numNPCs 'Display NPCs that should be in front of blocks
        For(A, 1, numNPCs)
        {
//            With NPC(A)
//                If .Effect = 0 Then
//                    If Not (.HoldingPlayer > 0 Or NPCIsAVine(.Type) = True Or .Type = 209 Or .Type = 282 Or .Type = 270 Or .Type = 160 Or .Type = 159 Or .Type = 8 Or .Type = 245 Or .Type = 93 Or .Type = 51 Or .Type = 52 Or .Type = 74 Or .Type = 256 Or .Type = 257 Or .Type = 56 Or .Type = 22 Or .Type = 49 Or .Type = 91) And Not (.Type = 45 And .Special = 0) And .standingOnPlayer = 0 And NPCForeground(.Type) = False And (.Generator = False Or LevelEditor = True) And .Type <> 179 And .Type <> 263 Then
//                        If Not NPCIsACoin(.Type) Then
//                            If vScreenCollision(Z, .Location) And .Hidden = False Then
//                                If .Type = 0 Then
//                                    NPC(A).Killed = 9
//                                    KillNPC A, 9
//                                ElseIf .Active = True Then
//                                    If NPCIsYoshi(.Type) = False Then
//                                        If NPCWidthGFX(.Type) = 0 Then
//                                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * .Location.Height, vbSrcAnd
//                                            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * .Location.Height, vbSrcPaint
//                                        Else
//                                            If .Type = 283 And .Special > 0 Then
//                                                If NPCWidthGFX(.Special) = 0 Then
//                                                    tempLocation.Width = NPCWidth(.Special)
//                                                    tempLocation.Height = NPCHeight(.Special)
//                                                Else
//                                                    tempLocation.Width = NPCWidthGFX(.Special)
//                                                    tempLocation.Height = NPCHeightGFX(.Special)
//                                                End If
//                                                tempLocation.X = .Location.X + .Location.Width / 2 - tempLocation.Width / 2
//                                                tempLocation.Y = .Location.Y + .Location.Height / 2 - tempLocation.Height / 2
//                                                B = EditorNPCFrame(Int(.Special), .Direction)
//                                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(.Type), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPCMask(.Special), 0, B * tempLocation.Height, vbSrcAnd
//                                                BitBlt myBackBuffer, vScreenX(Z) + tempLocation.X + NPCFrameOffsetX(.Type), vScreenY(Z) + tempLocation.Y, tempLocation.Width, tempLocation.Height, GFXNPC(.Special), 0, B * tempLocation.Height, vbSrcPaint
//                                            End If

//                                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + (NPCFrameOffsetX(.Type) * -.Direction) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPCMask(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcAnd
//                                            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + (NPCFrameOffsetX(.Type) * -.Direction) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPC(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcPaint
//                                        End If
//                                    Else
//                                        If .Type = 95 Then
//                                            B = 1
//                                        ElseIf .Type = 98 Then
//                                            B = 2
//                                        ElseIf .Type = 99 Then
//                                            B = 3
//                                        ElseIf .Type = 100 Then
//                                            B = 4
//                                        ElseIf .Type = 148 Then
//                                            B = 5
//                                        ElseIf .Type = 149 Then
//                                            B = 6
//                                        ElseIf .Type = 150 Then
//                                            B = 7
//                                        ElseIf .Type = 228 Then
//                                            B = 8
//                                        End If
//                                        Dim YoshiBX As Integer
//                                        Dim YoshiBY As Integer
//                                        Dim YoshiTX As Integer
//                                        Dim YoshiTY As Integer
//                                        Dim YoshiTFrame As Integer
//                                        Dim YoshiBFrame As Integer
//                                        YoshiBX = 0
//                                        YoshiBY = 0
//                                        YoshiTX = 20
//                                        YoshiTY = -32
//                                        YoshiBFrame = 6
//                                        YoshiTFrame = 0
//                                        If .Special = 0 Then
//                                            If FreezeNPCs = False Then .FrameCount = .FrameCount + 1
//                                            If .FrameCount >= 70 Then
//                                                If FreezeNPCs = False Then .FrameCount = 0
//                                            ElseIf .FrameCount >= 50 Then
//                                                YoshiTFrame = 3
//                                            End If
//                                        Else
//                                            If FreezeNPCs = False Then .FrameCount = .FrameCount + 1
//                                            If .FrameCount > 8 Then
//                                                YoshiBFrame = 0
//                                                .FrameCount = 0
//                                            ElseIf .FrameCount > 6 Then
//                                                YoshiBFrame = 1
//                                                YoshiTX = YoshiTX - 1
//                                                YoshiTY = YoshiTY + 2
//                                                YoshiBY = YoshiBY + 1
//                                            ElseIf .FrameCount > 4 Then
//                                                YoshiBFrame = 2
//                                                YoshiTX = YoshiTX - 2
//                                                YoshiTY = YoshiTY + 4
//                                                YoshiBY = YoshiBY + 2
//                                            ElseIf .FrameCount > 2 Then
//                                                YoshiBFrame = 1
//                                                YoshiTX = YoshiTX - 1
//                                                YoshiTY = YoshiTY + 2
//                                                YoshiBY = YoshiBY + 1
//                                            Else
//                                                YoshiBFrame = 0
//                                            End If
//                                            If FreezeNPCs = False Then .Special2 = .Special2 + 1
//                                            If .Special2 > 30 Then
//                                                YoshiTFrame = 0
//                                                If FreezeNPCs = False Then .Special2 = 0
//                                            ElseIf .Special2 > 10 Then
//                                                YoshiTFrame = 2
//                                            End If

//                                        End If
//                                        If YoshiBFrame = 6 Then
//                                            YoshiBY = YoshiBY + 10
//                                            YoshiTY = YoshiTY + 10
//                                        End If
//                                        If .Direction = 1 Then
//                                            YoshiTFrame = YoshiTFrame + 5
//                                            YoshiBFrame = YoshiBFrame + 7
//                                        Else
//                                            YoshiBX = -YoshiBX
//                                            YoshiTX = -YoshiTX
//                                        End If
//                                        'YoshiBX = YoshiBX + 4
//                                        'YoshiTX = YoshiTX + 4
//                                        'Yoshi's Body
//                                        BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + YoshiBX, vScreenY(Z) + .Location.Y + YoshiBY, 32, 32, GFXYoshiBMask(B), 0, 32 * YoshiBFrame, vbSrcAnd
//                                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + YoshiBX, vScreenY(Z) + .Location.Y + YoshiBY, 32, 32, GFXYoshiB(B), 0, 32 * YoshiBFrame, vbSrcPaint
//                                        'Yoshi's Head
//                                        BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + YoshiTX, vScreenY(Z) + .Location.Y + YoshiTY, 32, 32, GFXYoshiTMask(B), 0, 32 * YoshiTFrame, vbSrcAnd
//                                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + YoshiTX, vScreenY(Z) + .Location.Y + YoshiTY, 32, 32, GFXYoshiT(B), 0, 32 * YoshiTFrame, vbSrcPaint
//                                    End If
//                                End If
//                                If (.Reset(1) = True And .Reset(2) = True) Or .Active = True Or .Type = 57 Then
//                                    If .Active = False Then
//                                        .JustActivated = Z
//                                        If nPlay.Online = True Then
//                                            Netplay.sendData "2a" & A & "|" & nPlay.MySlot + 1 & LB
//                                            .JustActivated = nPlay.MySlot + 1
//                                        End If
//                                    End If
//                                    .TimeLeft = Physics.NPCTimeOffScreen
//                                    If NPCIsYoshi(.Type) Or NPCIsBoot(.Type) Or .Type = 9 Or .Type = 14 Or .Type = 22 Or .Type = 90 Or .Type = 153 Or .Type = 169 Or .Type = 170 Or .Type = 182 Or .Type = 183 Or .Type = 184 Or .Type = 185 Or .Type = 186 Or .Type = 187 Or .Type = 188 Or .Type = 195 Or .Type = 104 Then .TimeLeft = Physics.NPCTimeOffScreen * 20
//0
//                                    If nPlay.Online = True And nPlay.NPCWaitCount >= 10 And nPlay.Mode = 0 Then timeStr = timeStr & "2b" & A & LB
//                                    .Active = True
//                                End If
//                                .Reset(1) = False
//                                .Reset(2) = False
//                            Else
//                                .Reset(Z) = True
//                                If numScreens = 1 Then .Reset(2) = True
//                                If SingleCoop = 1 Then
//                                    .Reset(2) = True
//                                ElseIf SingleCoop = 2 Then
//                                    .Reset(1) = True
//                                End If
//                            End If
//                        End If
//                    End If
//                End If
//            End With
//        Next A
        }

//        'npc chat bubble
//        For A = 1 To numNPCs
//            With NPC(A)
//                If .Active = True And .Chat = True Then
//                    B = NPCHeightGFX(.Type) - .Location.Height
//                    If B < 0 Then B = 0
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X + .Location.Width / 2 - GFX.Chat.ScaleWidth / 2, vScreenY(Z) + .Location.Y - 30 - B, GFX.Chat.ScaleWidth, GFX.Chat.ScaleHeight, GFX.ChatMask.hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, vScreenX(Z) + .Location.X + .Location.Width / 2 - GFX.Chat.ScaleWidth / 2, vScreenY(Z) + .Location.Y - 30 - B, GFX.Chat.ScaleWidth, GFX.Chat.ScaleHeight, GFX.Chat.hdc, 0, 0, vbSrcPaint
//                End If
//            End With
//        Next A

//        For A = 1 To numPlayers 'The clown car
        For(A, 1, numPlayers)
        {
//            With Player(A)
//                If .Dead = False And .Immune2 = False And .TimeToLive = 0 And Not (.Effect = 3 Or .Effect = 5) And .Mount = 2 Then
//                     If .Character = 1 Then
//                         If .State = 1 Then
//                             Y = 24
//                         Else
//                             Y = 36
//                         End If
//                         BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + MarioFrameX((.State * 100) + (.Frame * .Direction)) - Physics.PlayerWidth(.Character, .State) / 2 + 64, vScreenY(Z) + .Location.Y + MarioFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY - Y, 99, .Location.Height - 20 - .MountOffsetY, GFXMarioMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                         If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + MarioFrameX((.State * 100) + (.Frame * .Direction)) - Physics.PlayerWidth(.Character, .State) / 2 + 64, vScreenY(Z) + .Location.Y + MarioFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY - Y, 99, .Location.Height - 20 - .MountOffsetY, GFXMario(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                         BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 64, vScreenY(Z) + .Location.Y + .Location.Height - 128, 128, 128, GFX.MountMask(.Mount).hdc, 0, 128 * .MountFrame, vbSrcAnd
//                         If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 64, vScreenY(Z) + .Location.Y + .Location.Height - 128, 128, 128, GFX.Mount(.Mount).hdc, 0, 128 * .MountFrame, vbSrcPaint
//                    ElseIf .Character = 2 Then
//                         If .State = 1 Then
//                             Y = 24
//                         Else
//                             Y = 38
//                         End If
//                         BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX((.State * 100) + (.Frame * .Direction)) - Physics.PlayerWidth(.Character, .State) / 2 + 64, vScreenY(Z) + .Location.Y + LuigiFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY - Y, 99, .Location.Height - 20 - .MountOffsetY, GFXLuigiMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                         If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LuigiFrameX((.State * 100) + (.Frame * .Direction)) - Physics.PlayerWidth(.Character, .State) / 2 + 64, vScreenY(Z) + .Location.Y + LuigiFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY - Y, 99, .Location.Height - 20 - .MountOffsetY, GFXLuigi(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                         BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 64, vScreenY(Z) + .Location.Y + .Location.Height - 128, 128, 128, GFX.MountMask(.Mount).hdc, 0, 128 * .MountFrame, vbSrcAnd
//                         If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 64, vScreenY(Z) + .Location.Y + .Location.Height - 128, 128, 128, GFX.Mount(.Mount).hdc, 0, 128 * .MountFrame, vbSrcPaint
//                    ElseIf .Character = 3 Then
//                         If .State = 1 Then
//                             Y = 24
//                         Else
//                             Y = 30
//                         End If
//                         BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + PeachFrameX((.State * 100) + (.Frame * .Direction)) - Physics.PlayerWidth(.Character, .State) / 2 + 64, vScreenY(Z) + .Location.Y + PeachFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY - Y, 99, .Location.Height - 20 - .MountOffsetY, GFXPeachMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                         If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + PeachFrameX((.State * 100) + (.Frame * .Direction)) - Physics.PlayerWidth(.Character, .State) / 2 + 64, vScreenY(Z) + .Location.Y + PeachFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY - Y, 99, .Location.Height - 20 - .MountOffsetY, GFXPeach(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                         BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 64, vScreenY(Z) + .Location.Y + .Location.Height - 128, 128, 128, GFX.MountMask(.Mount).hdc, 0, 128 * .MountFrame, vbSrcAnd
//                         If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 64, vScreenY(Z) + .Location.Y + .Location.Height - 128, 128, 128, GFX.Mount(.Mount).hdc, 0, 128 * .MountFrame, vbSrcPaint
//                    ElseIf .Character = 4 Then
//                         If .State = 1 Then
//                             Y = 24
//                         Else
//                             Y = 30
//                         End If
//                         BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + ToadFrameX((.State * 100) + (.Frame * .Direction)) - Physics.PlayerWidth(.Character, .State) / 2 + 64, vScreenY(Z) + .Location.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY - Y, 99, .Location.Height - 20 - .MountOffsetY, GFXToadMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                         If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + ToadFrameX((.State * 100) + (.Frame * .Direction)) - Physics.PlayerWidth(.Character, .State) / 2 + 64, vScreenY(Z) + .Location.Y + ToadFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY - Y, 99, .Location.Height - 20 - .MountOffsetY, GFXToad(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                         BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 64, vScreenY(Z) + .Location.Y + .Location.Height - 128, 128, 128, GFX.MountMask(.Mount).hdc, 0, 128 * .MountFrame, vbSrcAnd
//                         If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 64, vScreenY(Z) + .Location.Y + .Location.Height - 128, 128, 128, GFX.Mount(.Mount).hdc, 0, 128 * .MountFrame, vbSrcPaint
//                    ElseIf .Character = 5 Then
//                         Y = 30
//                         BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LinkFrameX((.State * 100) + (.Frame * .Direction)) - Physics.PlayerWidth(.Character, .State) / 2 + 64, vScreenY(Z) + .Location.Y + LinkFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY - Y, 99, .Location.Height - 20 - .MountOffsetY, GFXLinkMask(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcAnd
//                         If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + LinkFrameX((.State * 100) + (.Frame * .Direction)) - Physics.PlayerWidth(.Character, .State) / 2 + 64, vScreenY(Z) + .Location.Y + LinkFrameY((.State * 100) + (.Frame * .Direction)) + .MountOffsetY - Y, 99, .Location.Height - 20 - .MountOffsetY, GFXLink(.State), pfrX(100 + .Frame * .Direction), pfrY(100 + .Frame * .Direction), vbSrcPaint
//                         BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 64, vScreenY(Z) + .Location.Y + .Location.Height - 128, 128, 128, GFX.MountMask(.Mount).hdc, 0, 128 * .MountFrame, vbSrcAnd
//                         If ShadowMode = False Then BitBlt myBackBuffer, vScreenX(Z) + Int(.Location.X) + .Location.Width / 2 - 64, vScreenY(Z) + .Location.Y + .Location.Height - 128, 128, 128, GFX.Mount(.Mount).hdc, 0, 128 * .MountFrame, vbSrcPaint
//                     End If
//                End If
//            End With
//        Next A
        }


//        For A = 1 To numNPCs 'Put held NPCs on top
//            With NPC(A)
//                If (((.HoldingPlayer > 0 And Player(.HoldingPlayer).Effect <> 3) Or (.Type = 50 And .standingOnPlayer = 0) Or .Type = 17 And .CantHurt > 0) Or .Effect = 5) And Not .Type = 91 And Player(.HoldingPlayer).Dead = False Then
//                    If .Type = 263 Then
//                        DrawFrozenNPC Z, A
//                    ElseIf NPCIsYoshi(.Type) = False And .Type > 0 Then
//                        If NPCWidthGFX(.Type) = 0 Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * .Location.Height, vbSrcAnd
//                            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * .Location.Height, vbSrcPaint
//                        Else
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + (NPCFrameOffsetX(.Type) * -.Direction) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPCMask(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcAnd
//                            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + (NPCFrameOffsetX(.Type) * -.Direction) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPC(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcPaint
//                        End If
//                    End If
//                End If
//            End With
//        Next A




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
//            For A = LastBackground + 1 To numBackground 'Foreground objects
//                With Background(A)
//                    If vScreenCollision(Z, .Location) And .Hidden = False Then
//                        If BackgroundHasNoMask(.Type) Then
//                            'BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, GFXBackgroundWidth(.Type), GFXBackgroundHeight(.Type), GFXBackground(.Type), 0, 0, vbSrcCopy
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, GFXBackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackgroundMask(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y, BackgroundWidth(.Type), BackgroundHeight(.Type), GFXBackground(.Type), 0, BackgroundHeight(.Type) * BackgroundFrame(.Type), vbSrcPaint
//                        End If
//                    End If
//                End With
//            Next A
//        End If
        }

//        For A = 1 To numNPCs 'foreground NPCs
//            With NPC(A)
//                If .Effect = 0 Then
//                    If NPCForeground(.Type) = True And .HoldingPlayer = 0 And (.Generator = False Or LevelEditor = True) Then
//                        If Not NPCIsACoin(.Type) Then
//                            If vScreenCollision(Z, .Location) And .Hidden = False Then
//                                If .Active = True Then
//                                    If NPCWidthGFX(.Type) = 0 Then
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * .Location.Height, vbSrcAnd
//                                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * .Location.Height, vbSrcPaint
//                                    Else
//                                        BitBlt myBackBuffer, vScreenX(Z) + .Location.X + (NPCFrameOffsetX(.Type) * -.Direction) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPCMask(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcAnd
//                                        If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + (NPCFrameOffsetX(.Type) * -.Direction) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPC(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcPaint
//                                    End If
//                                End If
//                                If (.Reset(1) = True And .Reset(2) = True) Or .Active = True Then
//                                    If .Active = False Then
//                                        .JustActivated = Z
//                                        If nPlay.Online = True Then
//                                            Netplay.sendData "2a" & A & "|" & nPlay.MySlot + 1 & LB
//                                            .JustActivated = nPlay.MySlot + 1
//                                        End If
//                                    End If
//                                    .TimeLeft = Physics.NPCTimeOffScreen
//                                    If nPlay.Online = True And nPlay.NPCWaitCount >= 10 And nPlay.Mode = 0 Then timeStr = timeStr & "2b" & A & LB
//                                    .Active = True
//                                End If
//                                .Reset(1) = False
//                                .Reset(2) = False
//                            Else
//                                .Reset(Z) = True
//                                If numScreens = 1 Then .Reset(2) = True
//                                If SingleCoop = 1 Then
//                                    .Reset(2) = True
//                                ElseIf SingleCoop = 2 Then
//                                    .Reset(1) = True
//                                End If
//                            End If
//                        End If
//                    End If
//                End If
//            End With
//        Next A

//        For A = fBlock To lBlock 'Blocks in Front
//            With Block(A)
//                If BlockKills(.Type) Then
//                    If vScreenCollision(Z, .Location) And .Hidden = False Then
//                        If BlockHasNoMask(.Type) = True Then
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .ShakeY3, .Location.Width, .Location.Height, GFXBlock(.Type), 0, BlockFrame(.Type) * 32, vbSrcCopy
//                        Else
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .ShakeY3, .Location.Width, .Location.Height, GFXBlockMask(.Type), 0, BlockFrame(.Type) * 32, vbSrcAnd
//                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X, vScreenY(Z) + .Location.Y + .ShakeY3, .Location.Width, .Location.Height, GFXBlock(.Type), 0, BlockFrame(.Type) * 32, vbSrcPaint
//                        End If
//                    End If
//                End If
//            End With
//        Next A

//'effects on top
//        For A = 1 To numEffects
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


//        If LevelEditor = False Then 'Graphics for the main game.
//        'NPC Generators
//            For A = 1 To numNPCs
//                With NPC(A)
//                    If .Generator = True Then
//                        If vScreenCollision(Z, .Location) And .Hidden = False Then
//                            .GeneratorActive = True
//                        End If
//                    End If
//                End With
//            Next A
//            If vScreen(2).Visible = True Then
//                If vScreen(Z).Width = ScreenW Then
//                    If vScreen(Z).Top <> 0 Then
//                        BitBlt myBackBuffer, 0, 0, vScreen(Z).Width, 1, 0, 0, 0, vbWhiteness
//                    Else
//                        BitBlt myBackBuffer, 0, vScreen(Z).Height - 1, vScreen(Z).Width, 1, 0, 0, 0, vbWhiteness
//                    End If
//                Else
//                If vScreen(Z).Left <> 0 Then
//                        BitBlt myBackBuffer, 0, 0, 1, vScreen(Z).Height, 0, 0, 0, vbWhiteness
//                    Else
//                        BitBlt myBackBuffer, vScreen(Z).Width - 1, 0, 1, vScreen(Z).Height, 0, 0, 0, vbWhiteness
//                    End If
//                End If
//            End If

//'player names
//    If LevelEditor = False And nPlay.Online = True Then
//        For A = 1 To numPlayers
//            If Player(A).Dead = False And Player(A).TimeToLive = 0 Then
//                With Player(A)
//                    If A - 1 = nPlay.MySlot Then
//                        SuperPrint UCase(LocalNick), 3, vScreenX(Z) + .Location.X - Len(LocalNick) * 9 + .Location.Width / 2, vScreenY(Z) + .Location.Y - 32
//                    Else
//                        SuperPrint UCase(nPlay.Player(A - 1).Nick), 3, vScreenX(Z) + .Location.X - Len(nPlay.Player(A - 1).Nick) * 9 + .Location.Width / 2, vScreenY(Z) + .Location.Y - 32
//                    End If
//                End With
//            End If
//        Next A
//    End If



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
//                    With Player(A)
//                        If .ShowWarp > 0 Then
//                            Dim tempString As String
//                            If Warp(.ShowWarp).maxStars > 0 And .Mount <> 2 Then
//                                tempString = Warp(.ShowWarp).curStars & "/" & Warp(.ShowWarp).maxStars
//                                With Player(A)
//                                    BitBlt myBackBuffer, .Location.X + .Location.Width / 2 + vScreenX(Z) - Len(tempString) * 9, .Location.Y + .Location.Height - 96 + vScreenY(Z) + 1, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, .Location.X + .Location.Width / 2 + vScreenX(Z) - Len(tempString) * 9, .Location.Y + .Location.Height - 96 + vScreenY(Z) + 1, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                                    BitBlt myBackBuffer, .Location.X + .Location.Width / 2 + vScreenX(Z) - Len(tempString) * 9 - 20, .Location.Y + .Location.Height - 96 + vScreenY(Z), GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.InterfaceMask(5).hdc, 0, 0, vbSrcAnd
//                                    BitBlt myBackBuffer, .Location.X + .Location.Width / 2 + vScreenX(Z) - Len(tempString) * 9 - 20, .Location.Y + .Location.Height - 96 + vScreenY(Z), GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.Interface(5).hdc, 0, 0, vbSrcPaint
//                                    SuperPrint tempString, 3, .Location.X + .Location.Width / 2 + vScreenX(Z) - Len(tempString) * 9 + 18, .Location.Y + .Location.Height - 96 + vScreenY(Z)
//                                End With
//                            End If
//                        End If
//                    End With
//                Next A
            }

//                DrawInterface Z, numScreens
            DrawInterface(Z, numScreens);

//                For A = 1 To numNPCs 'Display NPCs that got dropped from the container
            For(A,1, numNPCs)
            {
//                    With NPC(A)
//                        If .Effect = 2 Then
//                            If .Effect2 Mod 3 <> 0 Then
//                                If vScreenCollision(Z, .Location) Then
//                                    If .Active = True Then
//                                        If NPCWidthGFX(.Type) = 0 Then
//                                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPCMask(.Type), 0, .Frame * .Location.Height, vbSrcAnd
//                                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width, .Location.Height, GFXNPC(.Type), 0, .Frame * .Location.Height, vbSrcPaint
//                                        Else
//                                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPCMask(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcAnd
//                                            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type) - NPCWidthGFX(.Type) / 2 + .Location.Width / 2, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) - NPCHeightGFX(.Type) + .Location.Height, NPCWidthGFX(.Type), NPCHeightGFX(.Type), GFXNPC(.Type), 0, .Frame * NPCHeightGFX(.Type), vbSrcPaint
//                                        End If
//                                    End If
//                                    If .Reset(Z) = True Or .Active = True Then
//                                        .TimeLeft = Physics.NPCTimeOffScreen
//                                        If nPlay.Online = True And nPlay.NPCWaitCount >= 10 And nPlay.Mode = 0 Then timeStr = timeStr & "2b" & A & LB
//                                        .Active = True
//                                    End If
//                                    .Reset(1) = False
//                                    .Reset(2) = False
//                                Else
//                                    .Reset(Z) = True
//                                End If
//                            End If
//                        End If
//                    End With
//                Next A
            }


//                If GamePaused = True Then
            if(GamePaused)
            {
//                    If MessageText = "" Then
//                        X = 0
//                        Y = 0
//                        If (DScreenType = 1 And Z = 2) Or (DScreenType = 2 And Z = 1) Then
//                            X = -400
//                        ElseIf (DScreenType = 6 And Z = 2) Or (DScreenType = 4 And Z = 2) Or (DScreenType = 3 And Z = 1) Then
//                            Y = -300
//                        End If
//                        BitBlt myBackBuffer, 210 + X, 200 + Y, 380, 200, 0, 0, 0, vbWhiteness
//                        If Cheater = False And (LevelSelect = True Or (StartLevel = FileName And NoMap = True)) Then
//                            SuperPrint "CONTINUE", 3, 272 + X, 257 + Y
//                            SuperPrint "SAVE & CONTINUE", 3, 272 + X, 292 + Y
//                            SuperPrint "SAVE & QUIT", 3, 272 + X, 327 + Y
//                            BitBlt myBackBuffer, 252 + X, 257 + (MenuCursor * 35) + Y, 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                            BitBlt myBackBuffer, 252 + X, 257 + (MenuCursor * 35) + Y, 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
//                        Else
//                            SuperPrint "CONTINUE", 3, 272 + 56 + X, 275 + Y
//                            SuperPrint "QUIT", 3, 272 + 56 + X, 310 + Y
//                            BitBlt myBackBuffer, 252 + 56 + X, 275 + (MenuCursor * 35) + Y, 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                            BitBlt myBackBuffer, 252 + 56 + X, 275 + (MenuCursor * 35) + Y, 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
//                        End If
//                    Else
//                        X = 0
//                        Y = 0
//                        If (DScreenType = 1 And Z = 2) Or (DScreenType = 2 And Z = 1) Then
//                            X = -400
//                        ElseIf (DScreenType = 6 And Z = 2) Or (DScreenType = 4 And Z = 2) Or (DScreenType = 3 And Z = 1) Then
//                            Y = -300
//                        End If

//                        SuperText = MessageText
//                        BoxY = 150
//                        BitBlt myBackBuffer, 400 - GFX.TextBox.ScaleWidth / 2 + X, BoxY + Y + Y, GFX.TextBox.ScaleWidth, 20, GFX.TextBox.hdc, 0, 0, vbSrcCopy
//                        BoxY = BoxY + 10
//                        tempBool = False
//                        Do
//                            B = 0
//                            For A = 1 To Len(SuperText)
//                                If Mid(SuperText, A, 1) = " " Or A = Len(SuperText) Then
//                                    If A < 28 Then
//                                        B = A
//                                    Else
//                                        Exit For
//                                    End If
//                                End If
//                            Next A
//                            If B = 0 Then B = A
//                            tempText = Left(SuperText, B)
//                            SuperText = Mid(SuperText, B + 1, Len(SuperText))
//                            BitBlt myBackBuffer, 400 - GFX.TextBox.ScaleWidth / 2 + X, BoxY + Y + Y, GFX.TextBox.ScaleWidth, 20, GFX.TextBox.hdc, 0, 20, vbSrcCopy
//                            If Len(SuperText) = 0 And tempBool = False Then
//                                SuperPrint tempText, 4, 162 + X + (27 * 9) - (Len(tempText) * 9), Y + BoxY
//                            Else
//                                SuperPrint tempText, 4, 162 + X, Y + BoxY
//                            End If
//                            BoxY = BoxY + 16
//                            tempBool = True
//                        Loop Until Len(SuperText) = 0
//                        BitBlt myBackBuffer, 400 - GFX.TextBox.ScaleWidth / 2 + X, BoxY + Y + Y, GFX.TextBox.ScaleWidth, 10, GFX.TextBox.hdc, 0, GFX.TextBox.ScaleHeight - 10, vbSrcCopy
//                    End If
//                End If
            }
//            ElseIf GameOutro = False Then
        } else if(!GameOutro) {

//                If MenuMode <> 1 And MenuMode <> 2 And MenuMode <> 4 Then worldCurs = 0
            if(MenuMode != 1 && MenuMode != 2 && MenuMode != 4)
                worldCurs = 0;
//Dim menuFix As Integer ' for Input Settings
//menuFix = -44
            int menuFix = 44;


//            BitBlt myBackBuffer, 0, 0, GFX.MenuGFX(1).ScaleWidth, GFX.MenuGFX(1).ScaleWidth, GFX.MenuGFXMask(1).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, 0, 0, GFX.MenuGFX(1).ScaleWidth, GFX.MenuGFX(1).ScaleWidth, GFX.MenuGFX(1).hdc, 0, 0, vbSrcPaint
            frmMain.renderTexture(0, 0, GFX.MenuGFX[1].w, GFX.MenuGFX[1].h, GFX.MenuGFX[1], 0, 0);
//            BitBlt myBackBuffer, ScreenW / 2 - GFX.MenuGFX(2).ScaleWidth / 2, 70, GFX.MenuGFX(2).ScaleWidth, GFX.MenuGFX(2).ScaleWidth, GFX.MenuGFXMask(2).hdc, 0, 0, vbSrcAnd
//            BitBlt myBackBuffer, ScreenW / 2 - GFX.MenuGFX(2).ScaleWidth / 2, 70, GFX.MenuGFX(2).ScaleWidth, GFX.MenuGFX(2).ScaleWidth, GFX.MenuGFX(2).hdc, 0, 0, vbSrcPaint
            frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[2].w / 2, 70,
                    GFX.MenuGFX[2].w, GFX.MenuGFX[2].h, GFX.MenuGFX[2], 0, 0);


//                BitBlt myBackBuffer, ScreenW / 2 - GFX.MenuGFX(3).ScaleWidth / 2, 576, GFX.MenuGFX(3).ScaleWidth, GFX.MenuGFX(3).ScaleWidth, GFX.MenuGFXMask(3).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, ScreenW / 2 - GFX.MenuGFX(3).ScaleWidth / 2, 576, GFX.MenuGFX(3).ScaleWidth, GFX.MenuGFX(3).ScaleWidth, GFX.MenuGFX(3).hdc, 0, 0, vbSrcPaint
            frmMain.renderTexture(ScreenW / 2 - GFX.MenuGFX[3].w / 2, 576,
                    GFX.MenuGFX[3].w, GFX.MenuGFX[3].h, GFX.MenuGFX[3], 0, 0);

//                If MenuMode = 0 Then 'Main Menu
            if(MenuMode == 0)
            {
//                    SuperPrint "1 PLAYER GAME", 3, 300, 350
                SuperPrint("1 PLAYER GAME", 3, 300, 350);
//                    SuperPrint "2 PLAYER GAME", 3, 300, 380
                SuperPrint("2 PLAYER GAME", 3, 300, 380);
//                    SuperPrint "BATTLE GAME", 3, 300, 410
                SuperPrint("BATTLE GAME", 3, 300, 410);
//                    SuperPrint "OPTIONS", 3, 300, 440
                SuperPrint("OPTIONS", 3, 300, 440);
//                    SuperPrint "EXIT", 3, 300, 470
                SuperPrint("EXIT", 3, 300, 470);
//                    BitBlt myBackBuffer, 300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
                frmMain.renderTexture(300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursor[0], 0, 0);
            }
//                ElseIf MenuMode = 100 Or MenuMode = 200 Or MenuMode = 300 Or MenuMode = 400 Or MenuMode = 500 Then   'Character select

//                    A = 0
//                    B = 0
//                    C = 0
//                    If blockCharacter(1) = False Then
//                        SuperPrint "MARIO GAME", 3, 300, 350
//                    Else
//                        A = A - 30
//                        If MenuCursor + 1 >= 1 Then B = B - 30
//                        If PlayerCharacter >= 1 Then C = C - 30
//                    End If
//                    If blockCharacter(2) = False Then
//                        SuperPrint "LUIGI GAME", 3, 300, 380 + A
//                    Else
//                        A = A - 30
//                        If MenuCursor + 1 >= 2 Then B = B - 30
//                        If PlayerCharacter >= 2 Then C = C - 30
//                    End If
//                    If blockCharacter(3) = False Then
//                        SuperPrint "PEACH GAME", 3, 300, 410 + A
//                    Else
//                        A = A - 30
//                        If MenuCursor + 1 >= 3 Then B = B - 30
//                        If PlayerCharacter >= 3 Then C = C - 30
//                    End If
//                    If blockCharacter(4) = False Then
//                        SuperPrint "TOAD GAME", 3, 300, 440 + A
//                    Else
//                        A = A - 30
//                        If MenuCursor + 1 >= 4 Then B = B - 30
//                        If PlayerCharacter >= 4 Then C = C - 30
//                    End If
//                    If blockCharacter(5) = False Then
//                        SuperPrint "LINK GAME", 3, 300, 470 + A
//                    Else
//                        A = A - 30
//                        If MenuCursor + 1 >= 5 Then B = B - 30
//                        If PlayerCharacter >= 5 Then C = C - 30
//                    End If
//                    If MenuMode = 300 Or MenuMode = 500 Then
//                        BitBlt myBackBuffer, 300 - 20, B + 350 + (MenuCursor * 30), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, 300 - 20, B + 350 + (MenuCursor * 30), 16, 16, GFX.MCursor(3).hdc, 0, 0, vbSrcPaint
//                        BitBlt myBackBuffer, 300 - 20, C + 350 + ((PlayerCharacter - 1) * 30), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, 300 - 20, C + 350 + ((PlayerCharacter - 1) * 30), 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
//                    Else
//                        BitBlt myBackBuffer, 300 - 20, B + 350 + (MenuCursor * 30), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                        BitBlt myBackBuffer, 300 - 20, B + 350 + (MenuCursor * 30), 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
//                    End If
//                ElseIf MenuMode = 1 Or MenuMode = 2 Or MenuMode = 4 Then 'World Select
//                    Dim tempStr As String
//                    minShow = 1
//                    maxShow = NumSelectWorld
//                    If NumSelectWorld > 5 Then
//                        minShow = worldCurs
//                        maxShow = minShow + 4

//                        If MenuCursor <= minShow - 1 Then worldCurs = worldCurs - 1
//                        If MenuCursor >= maxShow - 1 Then worldCurs = worldCurs + 1

//                        If worldCurs < 1 Then worldCurs = 1
//                        If worldCurs > NumSelectWorld - 4 Then worldCurs = NumSelectWorld - 4

//                        If maxShow >= NumSelectWorld Then
//                            maxShow = NumSelectWorld
//                            minShow = NumSelectWorld - 4
//                        End If

//                        minShow = worldCurs
//                        maxShow = minShow + 4

//                    End If


//                    For A = minShow To maxShow
//                        B = A - minShow + 1
//                        tempStr = UCase(SelectWorld(A).WorldName)
//                        SuperPrint tempStr, 3, 300, 320 + (B * 30)
//                    Next A

//                        If minShow > 1 Then
//                            BitBlt myBackBuffer, 400 - 8, 350 - 20, 16, 16, GFX.MCursorMask(1).hdc, 0, 0, vbSrcAnd
//                            BitBlt myBackBuffer, 400 - 8, 350 - 20, 16, 16, GFX.MCursor(1).hdc, 0, 0, vbSrcPaint
//                        End If
//                        If maxShow < NumSelectWorld Then
//                            BitBlt myBackBuffer, 400 - 8, 490, 16, 16, GFX.MCursorMask(2).hdc, 0, 0, vbSrcAnd
//                            BitBlt myBackBuffer, 400 - 8, 490, 16, 16, GFX.MCursor(2).hdc, 0, 0, vbSrcPaint
//                        End If


//                    B = MenuCursor - minShow + 1
//                    If B >= 0 And B < 5 Then
//                    BitBlt myBackBuffer, 300 - 20, 350 + (B * 30), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 300 - 20, 350 + (B * 30), 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
//                    End If
//                ElseIf MenuMode = 10 Or MenuMode = 20 Then 'Save Select
//                    For A = 1 To 3
//                        If SaveSlot(A) >= 0 Then
//                            SuperPrint "SLOT" & Str(A) & " ... " & SaveSlot(A), 3, 300, 320 + (A * 30)
//                            If SaveStars(A) > 0 Then
//                                BitBlt myBackBuffer, 560, 320 + (A * 30) + 1, GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.InterfaceMask(5).hdc, 0, 0, vbSrcAnd
//                                BitBlt myBackBuffer, 560, 320 + (A * 30) + 1, GFX.Interface(5).ScaleWidth, GFX.Interface(5).ScaleHeight, GFX.Interface(5).hdc, 0, 0, vbSrcPaint
//                                BitBlt myBackBuffer, 560 + 24, 320 + (A * 30) + 2, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.InterfaceMask(1).hdc, 0, 0, vbSrcAnd
//                                BitBlt myBackBuffer, 560 + 24, 320 + (A * 30) + 2, GFX.Interface(1).ScaleWidth, GFX.Interface(1).ScaleHeight, GFX.Interface(1).hdc, 0, 0, vbSrcPaint
//                                SuperPrint Str(SaveStars(A)), 3, 588, 320 + (A * 30)
//                            End If
//                        Else
//                            SuperPrint "SLOT" & Str(A) & " ... EMPTY", 3, 300, 320 + (A * 30)
//                        End If
//                    Next A
//                    BitBlt myBackBuffer, 300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
//                ElseIf MenuMode = 3 Then 'Options Menu
//                    SuperPrint "PLAYER 1 CONTROLS", 3, 300, 350
//                    SuperPrint "PLAYER 2 CONTROLS", 3, 300, 380
//                    If resChanged = True Then
//                        SuperPrint "WINDOWED MODE", 3, 300, 410
//                    Else
//                        SuperPrint "FULLSCREEN MODE", 3, 300, 410
//                    End If
//                    SuperPrint "VIEW CREDITS", 3, 300, 440
//                    BitBlt myBackBuffer, 300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 300 - 20, 350 + (MenuCursor * 30), 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
//                ElseIf MenuMode = 31 Or MenuMode = 32 Then
//                    If useJoystick(MenuMode - 30) = 0 Then
//                        SuperPrint "INPUT......KEYBOARD", 3, 300, 260 + menuFix
//                        SuperPrint "UP........." & CheckKey(Chr(conKeyboard(MenuMode - 30).Up)), 3, 300, 290 + menuFix
//                        SuperPrint "DOWN......." & CheckKey(Chr(conKeyboard(MenuMode - 30).Down)), 3, 300, 320 + menuFix
//                        SuperPrint "LEFT......." & CheckKey(Chr(conKeyboard(MenuMode - 30).Left)), 3, 300, 350 + menuFix
//                        SuperPrint "RIGHT......" & CheckKey(Chr(conKeyboard(MenuMode - 30).Right)), 3, 300, 380 + menuFix
//                        SuperPrint "RUN........" & CheckKey(Chr(conKeyboard(MenuMode - 30).Run)), 3, 300, 410 + menuFix
//                        SuperPrint "ALT RUN...." & CheckKey(Chr(conKeyboard(MenuMode - 30).AltRun)), 3, 300, 440 + menuFix
//                        SuperPrint "JUMP......." & CheckKey(Chr(conKeyboard(MenuMode - 30).Jump)), 3, 300, 470 + menuFix
//                        SuperPrint "ALT JUMP..." & CheckKey(Chr(conKeyboard(MenuMode - 30).AltJump)), 3, 300, 500 + menuFix
//                        SuperPrint "DROP ITEM.." & CheckKey(Chr(conKeyboard(MenuMode - 30).Drop)), 3, 300, 530 + menuFix
//                        SuperPrint "PAUSE......" & CheckKey(Chr(conKeyboard(MenuMode - 30).Start)), 3, 300, 560 + menuFix
//                    Else
//                        SuperPrint "INPUT......JOYSTICK " & useJoystick(MenuMode - 30), 3, 300, 260 + menuFix
//                        If conJoystick(MenuMode - 30).Run >= 0 Then
//                            SuperPrint "RUN........" & conJoystick(MenuMode - 30).Run, 3, 300, 290 + menuFix
//                        Else
//                            SuperPrint "RUN........_", 3, 300, 290 + menuFix
//                        End If
//                        If conJoystick(MenuMode - 30).AltRun >= 0 Then
//                            SuperPrint "ALT RUN...." & conJoystick(MenuMode - 30).AltRun, 3, 300, 320 + menuFix
//                        Else
//                            SuperPrint "ALT RUN...._", 3, 300, 320 + menuFix
//                        End If
//                        If conJoystick(MenuMode - 30).Jump >= 0 Then
//                            SuperPrint "JUMP......." & conJoystick(MenuMode - 30).Jump, 3, 300, 350 + menuFix
//                        Else
//                            SuperPrint "JUMP......._", 3, 300, 350 + menuFix
//                        End If
//                        If conJoystick(MenuMode - 30).AltJump >= 0 Then
//                            SuperPrint "ALT JUMP..." & conJoystick(MenuMode - 30).AltJump, 3, 300, 380 + menuFix
//                        Else
//                            SuperPrint "ALT JUMP..._", 3, 300, 380 + menuFix
//                        End If
//                        If conJoystick(MenuMode - 30).Drop >= 0 Then
//                            SuperPrint "DROP ITEM.." & conJoystick(MenuMode - 30).Drop, 3, 300, 410 + menuFix
//                        Else
//                            SuperPrint "DROP ITEM.._", 3, 300, 410 + menuFix
//                        End If
//                        If conJoystick(MenuMode - 30).Start >= 0 Then
//                            SuperPrint "PAUSE......" & conJoystick(MenuMode - 30).Start, 3, 300, 440 + menuFix
//                        Else
//                            SuperPrint "PAUSE......_", 3, 300, 440 + menuFix
//                        End If
//                    End If
//                    BitBlt myBackBuffer, 300 - 20, 260 + (MenuCursor * 30) + menuFix, 16, 16, GFX.MCursorMask(0).hdc, 0, 0, vbSrcAnd
//                    BitBlt myBackBuffer, 300 - 20, 260 + (MenuCursor * 30) + menuFix, 16, 16, GFX.MCursor(0).hdc, 0, 0, vbSrcPaint
//                End If


//                BitBlt myBackBuffer, MenuMouseX, MenuMouseY, GFX.ECursor(2).ScaleWidth, GFX.ECursor(2).ScaleHeight, GFX.ECursorMask(2).hdc, 0, 0, vbSrcAnd
//                BitBlt myBackBuffer, MenuMouseX, MenuMouseY, GFX.ECursor(2).ScaleWidth, GFX.ECursor(2).ScaleHeight, GFX.ECursor(2).hdc, 0, 0, vbSrcPaint
            frmMain.renderTexture(int(MenuMouseX), int(MenuMouseY),
                                  GFX.ECursor[2].w, GFX.ECursor[2].h, GFX.ECursor[2], 0, 0);


//            End If
        }
//            If PrintFPS > 0 Then
            if(PrintFPS > 0) {
//                SuperPrint Str(PrintFPS), 1, 8, 8
                SuperPrint(fmt::format_ne("%d", PrintFPS), 1, 8, 8);
//            End If
            }
//        End If

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
//        If GameOutro = True Then
//            BitBlt myBackBuffer, 0, 0, 800, CreditChop, myBackBuffer, 0, 0, vbWhiteness
//            BitBlt myBackBuffer, 0, 600 - CreditChop, 800, 600, myBackBuffer, 0, 0, vbWhiteness
//            DoCredits
//        End If
//        If LevelEditor = True Then
//            StretchBlt frmLevelWindow.vScreen(Z).hdc, 0, 0, frmLevelWindow.vScreen(Z).ScaleWidth, frmLevelWindow.vScreen(Z).ScaleHeight, myBackBuffer, 0, 0, 800, 600, vbSrcCopy
//        Else
//            If TestLevel = True And resChanged = False Then
//                If ScreenShake > 0 Then
//                    ScreenShake = ScreenShake - 1
//                    A = Int(Rnd * ScreenShake * 4) - ScreenShake * 2
//                    B = Int(Rnd * ScreenShake * 4) - ScreenShake * 2
//                    StretchBlt frmLevelWindow.vScreen(1).hdc, vScreen(Z).Left * (frmLevelWindow.vScreen(1).ScaleWidth / ScreenW) + A, vScreen(Z).Top * (frmLevelWindow.vScreen(1).ScaleHeight / ScreenH) + B, vScreen(Z).Width * (frmLevelWindow.vScreen(1).ScaleWidth / ScreenW), vScreen(Z).Height * (frmLevelWindow.vScreen(1).ScaleHeight / ScreenH), myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, vbSrcCopy
//                Else
//                    StretchBlt frmLevelWindow.vScreen(1).hdc, vScreen(Z).Left * (frmLevelWindow.vScreen(1).ScaleWidth / ScreenW), vScreen(Z).Top * (frmLevelWindow.vScreen(1).ScaleHeight / ScreenH), vScreen(Z).Width * (frmLevelWindow.vScreen(1).ScaleWidth / ScreenW), vScreen(Z).Height * (frmLevelWindow.vScreen(1).ScaleHeight / ScreenH), myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, vbSrcCopy
//                End If
//            Else
//                If ScreenShake > 0 Then
//                    ScreenShake = ScreenShake - 1
//                    A = Int(Rnd * ScreenShake * 4) - ScreenShake * 2
//                    B = Int(Rnd * ScreenShake * 4) - ScreenShake * 2
//                    StretchBlt frmMain.hdc, vScreen(Z).Left * (frmMain.ScaleWidth / ScreenW) + A, vScreen(Z).Top * (frmMain.ScaleHeight / ScreenH) + B, vScreen(Z).Width * (frmMain.ScaleWidth / ScreenW), vScreen(Z).Height * (frmMain.ScaleHeight / ScreenH), myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, vbSrcCopy
//                Else
//                    StretchBlt frmMain.hdc, vScreen(Z).Left * (frmMain.ScaleWidth / ScreenW), vScreen(Z).Top * (frmMain.ScaleHeight / ScreenH), vScreen(Z).Width * (frmMain.ScaleWidth / ScreenW), vScreen(Z).Height * (frmMain.ScaleHeight / ScreenH), myBackBuffer, 0, 0, vScreen(Z).Width, vScreen(Z).Height, vbSrcCopy
//                End If
//            End If
//        End If
//    Next Z
    }

//    If TakeScreen = True Then ScreenShot
    if(TakeScreen)
        ScreenShot();

//    'Update Coin Frames
//    CoinFrame2(1) = CoinFrame2(1) + 1
//    If CoinFrame2(1) >= 6 Then
//        CoinFrame2(1) = 0
//        CoinFrame(1) = CoinFrame(1) + 1
//        If CoinFrame(1) >= 4 Then CoinFrame(1) = 0
//    End If
//    CoinFrame2(2) = CoinFrame2(2) + 1
//    If CoinFrame2(2) >= 6 Then
//        CoinFrame2(2) = 0
//        CoinFrame(2) = CoinFrame(2) + 1
//        If CoinFrame(2) >= 7 Then CoinFrame(2) = 0
//    End If
//    CoinFrame2(3) = CoinFrame2(3) + 1
//    If CoinFrame2(3) >= 7 Then
//        CoinFrame2(3) = 0
//        CoinFrame(3) = CoinFrame(3) + 1
//        If CoinFrame(3) >= 4 Then CoinFrame(3) = 0
//    End If
//    If nPlay.Mode = 0 Then
//        If nPlay.NPCWaitCount >= 11 Then nPlay.NPCWaitCount = 0
//        nPlay.NPCWaitCount = nPlay.NPCWaitCount + 2
//        If timeStr <> "" Then
//            Netplay.sendData timeStr & LB
//        End If
//    End If
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
//        Do While Len(Words) > 0
        for(auto c : SuperWords)
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
//        If (vScreenCollision(Z, .Location) Or vScreenCollision(Z, newLoc(.Location.X - (NPCWidthGFX(.Type) - .Location.Width) / 2, .Location.Y, CDbl(NPCWidthGFX(.Type)), CDbl(NPCHeight(.Type))))) And .Hidden = False Then
//        'draw npc

//            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + 2, vScreenY(Z) + .Location.Y + 2, .Location.Width - 4, .Location.Height - 4, GFXNPCMask(.Special), 2, 2 + .Special2 * NPCHeight(.Special), vbSrcAnd
//            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + 2, vScreenY(Z) + .Location.Y + 2, .Location.Width - 4, .Location.Height - 4, GFXNPC(.Special), 2, 2 + .Special2 * NPCHeight(.Special), vbSrcPaint

//        'draw ice
//            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width - 6, .Location.Height - 6, GFXNPCMask(.Type), 0, 0, vbSrcAnd
//            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), .Location.Width - 6, .Location.Height - 6, GFXNPC(.Type), 0, 0, vbSrcPaint
//            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type) + .Location.Width - 6, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), 6, .Location.Height - 6, GFXNPCMask(.Type), 128 - 6, 0, vbSrcAnd
//            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type) + .Location.Width - 6, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type), 6, .Location.Height - 6, GFXNPC(.Type), 128 - 6, 0, vbSrcPaint
//            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) + .Location.Height - 6, .Location.Width - 6, 6, GFXNPCMask(.Type), 0, 128 - 6, vbSrcAnd
//            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type), vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) + .Location.Height - 6, .Location.Width - 6, 6, GFXNPC(.Type), 0, 128 - 6, vbSrcPaint
//            BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type) + .Location.Width - 6, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) + .Location.Height - 6, 6, 6, GFXNPCMask(.Type), 128 - 6, 128 - 6, vbSrcAnd
//            If .Shadow = False Then BitBlt myBackBuffer, vScreenX(Z) + .Location.X + NPCFrameOffsetX(.Type) + .Location.Width - 6, vScreenY(Z) + .Location.Y + NPCFrameOffsetY(.Type) + .Location.Height - 6, 6, 6, GFXNPC(.Type), 128 - 6, 128 - 6, vbSrcPaint
//        End If
//    End With
}
