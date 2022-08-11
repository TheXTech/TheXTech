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

#include "../globals.h"
#include "../frm_main.h"
#include "../gfx.h"
#include "../frame_timer.h"
#include "../graphics.h"
#include "../collision.h"
#include "../player.h"
#include "../compat.h"
#include "../config.h"
#include "../main/speedrunner.h"
#include "../main/trees.h"
#include "../main/screen_pause.h"
#include "../main/screen_connect.h"
#include "../main/screen_quickreconnect.h"
#include "../main/screen_textentry.h"
#include "../game_main.h"
#include "../main/world_globals.h"
#include "../core/render.h"
#include "../screen_fader.h"

#include "logic/world_map_fog.h"

#include "graphics/gfx_frame.h"

#include "gfx_special_frames.h"

#include <fmt_format_ne.h>

// based on Wohlstand's string-breaking algorithm (gfx_message.cpp)
static void s_DrawLevelName(const std::string& name, int lX, int bY, int w)
{
    const int charWidth = 16;
    const int lineHeight = 20;

    int lineStart = 0; // start of current line
    int lastWord = 0; // planned start of next line
    int numLines = 0; // n lines
    int maxChars = w/charWidth+1; // 27 by default

    // PASS ONE: determine the number of lines
    while(lineStart < int(name.size()))
    {
        lastWord = lineStart;

        for(int i = lineStart + 1; i <= lineStart+maxChars; i++)
        {
            auto c = name[size_t(i) - 1];

            if((lastWord == lineStart && i == lineStart+maxChars) || i == int(name.size()) || c == '\n')
            {
                lastWord = i;
                break;
            }
            else if(c == ' ')
            {
                lastWord = i;
            }
        }

        numLines ++;
        lineStart = lastWord;
    }

    // PASS TWO: draw the lines
    int Y = bY - (numLines - 1) * 20;
    lineStart = 0; // start of current line

    while(lineStart < int(name.size()))
    {
        lastWord = lineStart;
        for(int i = lineStart + 1; i <= lineStart+maxChars; i++)
        {
            auto c = name[size_t(i) - 1];

            if((lastWord == lineStart && i == lineStart+maxChars) || i == int(name.size()) || c == '\n')
            {
                lastWord = i;
                break;
            }
            else if(c == ' ')
            {
                lastWord = i;
            }
        }

        SuperPrint(size_t(lastWord) - size_t(lineStart), name.c_str() + size_t(lineStart),
            2,
            lX,
            Y);

        lineStart = lastWord;
        Y += lineHeight;
    }
}

// draws GFX to screen when on the world map/world map editor
void UpdateGraphics2(bool skipRepaint)
{
    if(!GameIsActive)
        return;

#ifdef USE_RENDER_BLOCKING
    if(XRender::renderBlocked())
        return;
#endif

    float c = ShadowMode ? 0.f : 1.f;
    cycleNextInc();

    if(FrameSkip && frameSkipNeeded())
        return;

    XRender::setTargetTexture();

    frameNextInc();
    frameRenderStart();

    g_stats.reset();

    int A = 0;
    int B = 0;
    const int Z = 1;
    int WPHeight = 0;
//    Location_t tempLocation;
    //Z = 1;

    vScreen[Z].Left = 0;
    vScreen[Z].Top = 0;
    vScreen[Z].Width = ScreenW;
    vScreen[Z].Height = ScreenH;
    CenterScreens();

    SpecialFrames();
    SceneFrame2[1] += 1;
    if(SceneFrame2[1] >= 12)
    {
        SceneFrame2[1] = 0;
        SceneFrame[1] += 1;
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
    SceneFrame2[27] += 1;
    if(SceneFrame2[27] >= 8)
    {
        SceneFrame2[27] = 0;
        SceneFrame[27] += 1;
        if(SceneFrame[27] >= 12)
            SceneFrame[27] = 0;
        SceneFrame[28] = SceneFrame[27];
        SceneFrame[29] = SceneFrame[27];
        SceneFrame[30] = SceneFrame[27];
    }
    SceneFrame2[33] += 1;
    if(SceneFrame2[33] >= 4)
    {
        SceneFrame2[33] = 0;
        SceneFrame[33] = SceneFrame[27] + 1;
        if(SceneFrame[33] >= 14)
            SceneFrame[33] = 0;
        SceneFrame[34] = SceneFrame[33];
    }
    SceneFrame2[62] += 1;
    if(SceneFrame2[62] >= 8)
    {
        SceneFrame2[62] = 0;
        SceneFrame[62] += 1;
        if(SceneFrame[62] >= 8)
            SceneFrame[62] = 0;
        SceneFrame[63] = SceneFrame[62];
    }
    LevelFrame2[2] += 1;
    if(LevelFrame2[2] >= 6)
    {
        LevelFrame2[2] = 0;
        LevelFrame[2] += 1;
        if(LevelFrame[2] >= 6)
            LevelFrame[2] = 0;
        LevelFrame[9] = LevelFrame[2];
        LevelFrame[13] = LevelFrame[2];
        LevelFrame[14] = LevelFrame[2];
        LevelFrame[15] = LevelFrame[2];
        LevelFrame[31] = LevelFrame[2];
        LevelFrame[32] = LevelFrame[2];
    }
    LevelFrame2[8] += 1;
    if(LevelFrame2[8] >= 12)
    {
        LevelFrame2[8] = 0;
        LevelFrame[8] += 1;
        if(LevelFrame[8] >= 4)
            LevelFrame[8] = 0;
    }
    LevelFrame2[12] += 1;
    if(LevelFrame2[12] >= 8)
    {
        LevelFrame2[12] = 0;
        LevelFrame[12] += 1;
        if(LevelFrame[12] >= 2)
            LevelFrame[12] = 0;
    }
    LevelFrame2[25] += 1;
    if(LevelFrame2[25] >= 8)
    {
        LevelFrame2[25] = 0;
        LevelFrame[25] += 1;
        if(LevelFrame[25] >= 4)
            LevelFrame[25] = 0;
        LevelFrame[26] = LevelFrame[25];
    }
    TileFrame2[14] += 1;
    if(TileFrame2[14] >= 14)
    {
        TileFrame2[14] = 0;
        TileFrame[14] += 1;
        if(TileFrame[14] >= 4)
            TileFrame[14] = 0;
        TileFrame[27] = TileFrame[14];
        TileFrame[241] = TileFrame[14];
    }

//    if(WorldEditor == true)
//    {
//        XRender::renderTexture(0, 0, ScreenW, ScreenH, 0, 0, 0);
//    }
//    else
//    {
//        XRender::renderTexture(0, 0, ScreenW, ScreenH, 0, 0, 0);
//    }
    XRender::clearBuffer();
    DrawBackdrop();

    XRender::setViewport(vScreen[Z].ScreenLeft, vScreen[Z].ScreenTop,
        vScreen[Z].Width, vScreen[Z].Height);
    XRender::renderRect(0, 0, vScreen[Z].Width, vScreen[Z].Height, 0, 0, 0);

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
    double sLeft, sTop, sRight, sBottom, sW, sH;
    double margin, marginTop, marginBottom;
    {

        sW = vScreen[Z].Width;
        sH = vScreen[Z].Height;

        margin = 66;
        marginTop = 130;
        marginBottom = 66;

        if(WorldEditor)
        {
            marginBottom = 0;
            marginTop = 0;
        }
        else if(sH < 400)
        {
            marginBottom = 24;
            marginTop = 72;
        }
        else if(sH < 500)
        {
            marginBottom = 32;
            marginTop = 96;
        }

        if(WorldEditor)
        {
            margin = 0;
        }
        else if(sW < 400)
        {
            margin = 24;
        }
        else if(sW < 600)
        {
            margin = 32;
        }
        else if(sW < 800)
        {
            margin = 48;
        }

        if(WorldEditor)
        {
            sLeft = -vScreenX[1];
            sTop = -vScreenY[1];
            sRight = -vScreenX[1] + vScreen[1].Width;
            sBottom = -vScreenY[1] + vScreen[1].Height;
        }
        else
        {
            sLeft = -vScreenX[1] + margin - 2;
            sTop = -vScreenY[1] + (marginTop - 34);
            sRight = -vScreenX[1] + vScreen[1].Width - (margin - 2);
            sBottom = -vScreenY[1] + vScreen[1].Height - (marginBottom - 2);
        }

        Location_t sView;
        sView.X = sLeft;
        sView.Y = sTop;
        sView.Width = sRight - sLeft;
        sView.Height = sBottom - sTop;

        //for(A = 1; A <= numTiles; A++)
        for(Tile_t* t : treeWorldTileQuery(sLeft, sTop, sRight, sBottom, true))
        {
            Tile_t &tile = *t;
            SDL_assert(IF_INRANGE(tile.Type, 1, maxTileType));

            g_stats.checkedTiles++;
            if(CheckCollision(sView, tile.Location))
            {
                g_stats.renderedTiles++;
//                XRender::renderTexture(vScreenX[Z] + Tile[A].Location.X, vScreenY[Z] + Tile[A].Location.Y, Tile[A].Location.Width, Tile[A].Location.Height, GFXTile[Tile[A].Type], 0, TileHeight[Tile[A].Type] * TileFrame[Tile[A].Type]);
                XRender::renderTexture(vScreenX[Z] + tile.Location.X,
                                      vScreenY[Z] + tile.Location.Y,
                                      tile.Location.Width,
                                      tile.Location.Height,
                                      GFXTileBMP[tile.Type], 0, TileHeight[tile.Type] * TileFrame[tile.Type]);
            }
        }

        //for(A = 1; A <= numScenes; A++)
        for(Scene_t* t : treeWorldSceneQuery(sLeft, sTop, sRight, sBottom, true))
        {
            Scene_t &scene = *t;
            SDL_assert(IF_INRANGE(scene.Type, 1, maxSceneType));

            g_stats.checkedScenes++;
            if(CheckCollision(sView, scene.Location) && (WorldEditor || scene.Active))
            {
                g_stats.renderedScenes++;
//                XRender::renderTexture(vScreenX[Z] + scene.Location.X, vScreenY[Z] + scene.Location.Y, scene.Location.Width, scene.Location.Height, GFXSceneMask[scene.Type], 0, SceneHeight[scene.Type] * SceneFrame[scene.Type]);
//                XRender::renderTexture(vScreenX[Z] + scene.Location.X, vScreenY[Z] + scene.Location.Y, scene.Location.Width, scene.Location.Height, GFXScene[scene.Type], 0, SceneHeight[scene.Type] * SceneFrame[scene.Type]);
                XRender::renderTexture(vScreenX[Z] + scene.Location.X,
                                      vScreenY[Z] + scene.Location.Y,
                                      scene.Location.Width, scene.Location.Height,
                                      GFXSceneBMP[scene.Type], 0, SceneHeight[scene.Type] * SceneFrame[scene.Type]);
            }
        }

        //for(A = 1; A <= numWorldPaths; A++)
        for(WorldPath_t* t : treeWorldPathQuery(sLeft, sTop, sRight, sBottom, true))
        {
            WorldPath_t &path = *t;
            SDL_assert(IF_INRANGE(path.Type, 1, maxPathType));

            g_stats.checkedPaths++;
            if(CheckCollision(sView, path.Location) && (WorldEditor || path.Active))
            {
                g_stats.renderedPaths++;
//                XRender::renderTexture(vScreenX[Z] + path.Location.X, vScreenY[Z] + path.Location.Y, path.Location.Width, path.Location.Height, GFXPathMask[path.Type], 0, 0);
//                XRender::renderTexture(vScreenX[Z] + path.Location.X, vScreenY[Z] + path.Location.Y, path.Location.Width, path.Location.Height, GFXPath[path.Type], 0, 0);
                XRender::renderTexture(vScreenX[Z] + path.Location.X,
                                      vScreenY[Z] + path.Location.Y,
                                      path.Location.Width, path.Location.Height,
                                      GFXPathBMP[path.Type], 0, 0);
            }
        }

        //for(A = 1; A <= numWorldLevels; A++)
        for(WorldLevel_t* t : treeWorldLevelQuery(sLeft, sTop, sRight, sBottom, true))
        {
            WorldLevel_t &level = *t;
            SDL_assert(IF_INRANGE(level.Type, 0, maxLevelType));

            g_stats.checkedLevels++;
            if(CheckCollision(sView, level.Location) && (WorldEditor || level.Active))
            {
                g_stats.renderedLevels++;
                if(level.Path)
                {
                    XRender::renderTexture(vScreenX[Z] + level.Location.X,
                                          vScreenY[Z] + level.Location.Y,
                                          level.Location.Width,
                                          level.Location.Height,
                                          GFXLevelBMP[0], 0, 0);
                }
                if(level.Path2)
                {
                    XRender::renderTexture(vScreenX[Z] + level.Location.X - 16,
                                          vScreenY[Z] + 8 + level.Location.Y,
                                          64, 32,
                                          GFXLevelBMP[29], 0, 0);
                }
                if(GFXLevelBig[level.Type])
                {
                    XRender::renderTexture(vScreenX[Z] + level.Location.X - (GFXLevelWidth[level.Type] - 32) / 2.0,
                                          vScreenY[Z] + level.Location.Y - GFXLevelHeight[level.Type] + 32,
                                          GFXLevelWidth[level.Type], GFXLevelHeight[level.Type],
                                          GFXLevelBMP[level.Type], 0, 32 * LevelFrame[level.Type]);
                }
                else
                {
                    XRender::renderTexture(vScreenX[Z] + level.Location.X,
                                          vScreenY[Z] + level.Location.Y,
                                          level.Location.Width, level.Location.Height,
                                          GFXLevelBMP[level.Type], 0, 32 * LevelFrame[level.Type]);
                }
            }
        }
    }

    if(WorldEditor)
    {
        for(A = 1; A <= numEffects; A++)
        {
            if(vScreenCollision(Z, Effect[A].Location))
            {
                XRender::renderTexture(vScreenX[Z] + Effect[A].Location.X,
                    vScreenY[Z] + Effect[A].Location.Y,
                    Effect[A].Location.Width, Effect[A].Location.Height,
                    GFXEffect[Effect[A].Type], 0, Effect[A].Frame * EffectHeight[Effect[A].Type]);
            }
        }

        for(WorldMusic_t* t : treeWorldMusicQuery(sLeft, sTop, sRight, sBottom, true))
        {
            WorldMusic_t &music = *t;
            if(vScreenCollision(Z, music.Location))
            {
                XRender::renderRect(vScreenX[Z] + music.Location.X, vScreenY[Z] + music.Location.Y, 32, 32,
                    1.f, 0.f, 1.f, 1.f, false);
                SuperPrint(std::to_string(music.Type), 1, vScreenX[Z] + music.Location.X + 2, vScreenY[Z] + music.Location.Y + 2);
            }
        }

        DrawEditorWorld();
    }
    else
    { // NOT AN EDITOR!!!
        if(WorldPlayer[1].Type == 0)
            WorldPlayer[1].Type = 1;

        WorldPlayer[1].Type = Player[1].Character;

        switch(WorldPlayer[1].Type)
        {
        case 3:
            WPHeight = 44;
            break;
        case 4:
            WPHeight = 40;
            break;
        default:
            WPHeight = 32;
            break;
        }

//        XRender::renderTexture(vScreenX[Z] + WorldPlayer[1].Location.X, vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayerMask[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);
//        XRender::renderTexture(vScreenX[Z] + WorldPlayer[1].Location.X, vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayer[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);
        XRender::renderTexture(vScreenX[Z] + WorldPlayer[1].Location.X,
                              vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight,
                              WorldPlayer[1].Location.Width, WPHeight,
                              GFXPlayerBMP[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);

        // render the fog-of-war effect
        if(g_worldMapFog.m_active && !WalkAnywhere)
        {
            // render fog map
            for(int row = 0; row < g_worldMapFog.m_map_rows; row++)
            {
                double y = g_worldMapFog.m_map_top + g_worldMapFog.m_tile_size * row + vScreenY[Z];
                if(y + g_worldMapFog.m_tile_size < marginTop || y >= sH - marginBottom)
                    continue;

                for(int col = 0; col < g_worldMapFog.m_map_cols; col++)
                {
                    double x = g_worldMapFog.m_map_left + g_worldMapFog.m_tile_size * col + vScreenX[Z];
                    if(x + g_worldMapFog.m_tile_size < margin || x >= sW - margin)
                        continue;

                    int8_t fog_alpha = g_worldMapFog.m_fog_alpha[row * g_worldMapFog.m_map_cols + col];
                    if(fog_alpha != 0)
                    {
                        if(GFX.WorldMapFog.inited)
                        {
                            DrawTextureTiled(x,
                                y,
                                g_worldMapFog.m_tile_size,
                                g_worldMapFog.m_tile_size,
                                GFX.WorldMapFog,
                                0, 0, -1, -1,
                                g_worldMapFog.m_map_left + g_worldMapFog.m_tile_size * col,
                                g_worldMapFog.m_map_top + g_worldMapFog.m_tile_size * row,
                                fog_alpha / (float)g_worldMapFog.m_fog_levels);
                        }
                        else
                        {
                            XRender::renderRect(x,
                                y,
                                g_worldMapFog.m_tile_size,
                                g_worldMapFog.m_tile_size,
                                0.8f, 0.8f, 0.8f, fog_alpha / (float)g_worldMapFog.m_fog_levels);
                        }
                    }
                }
            }
        }

        if(!WorldPlayer[1].LevelName.empty())
        {
             auto &s = WorldPlayer[1].stars;

             if(s.max > 0 && s.displayPolicy > Compatibility_t::STARS_DONT_SHOW)
             {
                 std::string label;

                 if(s.displayPolicy >= Compatibility_t::STARS_SHOW_COLLECTED_AND_AVAILABLE)
                     label = fmt::format_ne("{0}/{1}", s.cur, s.max);
                 else
                     label = fmt::format_ne("{0}", s.cur);

                 int len = SuperTextPixLen(label, 3);
                 int totalLen = len + GFX.Interface[1].w + GFX.Interface[5].w + 8 + 4;
                 int x = vScreenX[Z] + WorldPlayer[1].Location.X + (WorldPlayer[1].Location.Width / 2) - (totalLen / 2);
                 int y = vScreenY[Z] + WorldPlayer[1].Location.Y - 32;

                 XRender::renderTexture(x, y,
                                       GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                 XRender::renderTexture(x + GFX.Interface[5].w + 8, y,
                                       GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                 SuperPrint(label, 3, x + GFX.Interface[1].w + GFX.Interface[5].w + 8 + 4, y);
             }
        }

//        XRender::renderTexture(0, 0, 800, 130, GFX.Interface[4], 0, 0);
        if(GFX.WorldMapFrame_Tile.inited && (!GFX.Interface[4].inited || !GFX.isCustom(37) || GFX.isCustom(69)))
        {
            bool border_valid = GFX.WorldMapFrame_Border.inited && (!GFX.isCustom(69) || GFX.isCustom(70));

            XRender::resetViewport();

            RenderFrame(newLoc(0, 0, ScreenW, ScreenH), newLoc(vScreen[Z].ScreenLeft + margin, vScreen[Z].ScreenTop + marginTop, sW - margin - margin, sH - marginTop - marginBottom),
                GFX.WorldMapFrame_Tile, border_valid ? &GFX.WorldMapFrame_Border : nullptr, &g_worldMapFrameBorderInfo);

            XRender::setViewport(vScreen[Z].ScreenLeft, vScreen[Z].ScreenTop,
                vScreen[Z].Width, vScreen[Z].Height);
        }
        else
        {
            // render a legacy background, in MANY careful segments...

            // top-left
            XRender::renderTexture(0, 0, margin, marginTop, GFX.Interface[4], 66-margin, 130-marginTop);
            // top
            A = GFX.Interface[4].w-66-66;
            for(B = 0; B < (sW-margin*2)/A+1; B++)
                XRender::renderTexture(margin+B*A, 0, A, marginTop, GFX.Interface[4], 66, 130-marginTop);
            // top-right
            XRender::renderTexture(sW-margin, 0, margin, marginTop+20, GFX.Interface[4], GFX.Interface[4].w-66, 130-marginTop);
            // left
            A = GFX.Interface[4].h-130-66;
            for(B = 0; B < (sH-marginTop-marginBottom)/A+1; B++)
                XRender::renderTexture(0, marginTop+B*A, margin, A, GFX.Interface[4], 66-margin, 130);
            // right
            A = GFX.Interface[4].h-(130+20)-66;
            for(B = 0; B < (sH-(marginTop+20)-marginBottom)/A+1; B++)
                XRender::renderTexture(sW-margin, (marginTop+20)+B*A, margin, A, GFX.Interface[4], GFX.Interface[4].w-66, 150);
            // bottom-left
            XRender::renderTexture(0, sH-marginBottom, margin+34, marginBottom, GFX.Interface[4], 66-margin, GFX.Interface[4].h-66);
            // bottom
            A = GFX.Interface[4].w-100-66;
            for(B = 0; B < (sW-(margin+34)-margin)/A+1; B++)
                XRender::renderTexture((margin+34)+B*A, sH-marginBottom, A, marginBottom, GFX.Interface[4], 100, GFX.Interface[4].h-66);
            // bottom-right
            XRender::renderTexture(sW-margin, sH-marginBottom, margin, marginBottom, GFX.Interface[4], GFX.Interface[4].w-66, GFX.Interface[4].h-66);
        }


        int pX, pY;
        pY = marginTop - 6;

        for(A = 1; A <= numPlayers; A++)
        {
            pX = margin * 1.5 - 64 + 48 * A;

            Player[A].Direction = -1;
            Player[A].Location.SpeedY = 0;
            Player[A].Location.SpeedX = -1;
            Player[A].Controls.Left = false;
            Player[A].Controls.Right = false;
            if(Player[A].Duck)
                UnDuck(Player[A]);
            PlayerFrame(Player[A]);

            if(Player[A].Mount == 3)
            {
                if(Player[A].MountType == 0)
                    Player[A].MountType = 1;
                B = Player[A].MountType;
                // Yoshi's Body
                XRender::renderTexture(pX + Player[A].YoshiBX, pY - Player[A].Location.Height + Player[A].YoshiBY,
                                      32, 32, GFXYoshiBBMP[B], 0, 32 * Player[A].YoshiBFrame, c, c, c);

                // Yoshi's Head
                XRender::renderTexture(pX + Player[A].YoshiTX,
                                      pY - Player[A].Location.Height + Player[A].YoshiTY,
                                      32, 32, GFXYoshiTBMP[B], 0, 32 * Player[A].YoshiTFrame, c, c, c);
            }

            if(Player[A].Character == 1)
            {
                if(Player[A].Mount == 0 || Player[A].Mount == 3)
                {
                    XRender::renderTexture(pX + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          pY - Player[A].Location.Height + MarioFrameY[(Player[A].State * 100) +
                                            (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY,
                                          100, 100,
                                          GFXMarioBMP[Player[A].State],
                                          pfrX(Player[A].Frame * Player[A].Direction),
                                          pfrY(Player[A].Frame * Player[A].Direction), c, c, c);
                }
                else if(Player[A].Mount == 1)
                {
                    int height;
                    if(Player[A].State == 1)
                        height = Player[A].Location.Height - MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    else
                        height = Player[A].Location.Height - MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                    XRender::renderTexture(pX + MarioFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          pY - Player[A].Location.Height + MarioFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          100,
                                          height,
                                          GFXMarioBMP[Player[A].State],
                                          pfrX(Player[A].Frame * Player[A].Direction),
                                          pfrY(Player[A].Frame * Player[A].Direction), c, c, c);

                    XRender::renderTexture(pX + Player[A].Location.Width / 2.0 - 16, pY - 30, 32, 32,
                                          GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount += 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;

                        XRender::renderTexture(pX + Player[A].Location.Width / 2.0 - 16 + 20, pY - 30 - 10, 32, 32,
                                              GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame);
                    }
                }
            }
            else if(Player[A].Character == 2)
            {
                if(Player[A].Mount == 0 || Player[A].Mount == 3)
                {
                    XRender::renderTexture(pX + LuigiFrameX[(Player[A].State * 100) +
                                            (Player[A].Frame * Player[A].Direction)],
                                          pY - Player[A].Location.Height + LuigiFrameY[(Player[A].State * 100) +
                                            (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY, 100, 100,
                                          GFXLuigiBMP[Player[A].State],
                                          pfrX(Player[A].Frame * Player[A].Direction),
                                          pfrY(Player[A].Frame * Player[A].Direction), c, c, c);
                }
                else if(Player[A].Mount == 1)
                {
                    int height;
                    if(Player[A].State == 1)
                        height = Player[A].Location.Height - LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    else
                        height = Player[A].Location.Height - LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                    XRender::renderTexture(pX + LuigiFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          pY - Player[A].Location.Height + LuigiFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          100,
                                          height,
                                          GFXLuigiBMP[Player[A].State],
                                          pfrX(Player[A].Frame * Player[A].Direction),
                                          pfrY(Player[A].Frame * Player[A].Direction), c, c, c);

                    XRender::renderTexture(pX + Player[A].Location.Width / 2.0 - 16, pY - 30, 32, 32, GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount += 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;

                        XRender::renderTexture(pX + Player[A].Location.Width / 2.0 - 16 + 20, pY - 30 - 10, 32, 32, GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, c, c, c);
                    }
                }
            }
            else if(Player[A].Character == 3)
            {
                if(Player[A].Mount == 0 || Player[A].Mount == 3)
                {
                    XRender::renderTexture(pX + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          pY - Player[A].Location.Height + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY,
                                          100, 100,
                                          GFXPeachBMP[Player[A].State],
                                          pfrX(Player[A].Frame * Player[A].Direction),
                                          pfrY(Player[A].Frame * Player[A].Direction), c, c, c);
                }
                else if(Player[A].Mount == 1)
                {
                    int height;
                    if(Player[A].State == 1)
                        height = Player[A].Location.Height - PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                    else
                        height = Player[A].Location.Height - PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 30;
                    XRender::renderTexture(pX + PeachFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          pY - Player[A].Location.Height + PeachFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          100,
                                          height,
                                          GFXPeachBMP[Player[A].State],
                                          pfrX(Player[A].Frame * Player[A].Direction),
                                          pfrY(Player[A].Frame * Player[A].Direction), c, c, c);

                    XRender::renderTexture(pX + Player[A].Location.Width / 2.0 - 16, pY - 30, 32, 32, GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount += 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;

                        XRender::renderTexture(pX + Player[A].Location.Width / 2.0 - 16 + 20, pY - 30 - 10, 32, 32, GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, c, c, c);
                    }
                }
            }
            else if(Player[A].Character == 4)
            {
                if(Player[A].Mount == 0 || Player[A].Mount == 3)
                {
                    XRender::renderTexture(pX + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                          pY - Player[A].Location.Height + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] + Player[A].MountOffsetY,
                                          100, 100,
                                          GFXToadBMP[Player[A].State],
                                          pfrX(Player[A].Frame * Player[A].Direction),
                                          pfrY(Player[A].Frame * Player[A].Direction), c, c, c);
                }
                else if(Player[A].Mount == 1)
                {
                    if(Player[A].State == 1)
                    {
                        XRender::renderTexture(pX + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                              6 + pY - Player[A].Location.Height + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                              100,
                                              Player[A].Location.Height - 24,
                                              GFXToadBMP[Player[A].State],
                                              pfrX(Player[A].Frame * Player[A].Direction),
                                              pfrY(Player[A].Frame * Player[A].Direction), c, c, c);
                    }
                    else
                    {
                        int height;
                        if(Player[A].State == 1)
                            height = Player[A].Location.Height - ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)];
                        else
                            height = Player[A].Location.Height - ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)] - 26;
                        XRender::renderTexture(pX + ToadFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                              pY - Player[A].Location.Height + ToadFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                              100,
                                              height,
                                              GFXToadBMP[Player[A].State],
                                              pfrX(Player[A].Frame * Player[A].Direction),
                                              pfrY(Player[A].Frame * Player[A].Direction), c, c, c);
                    }
                    XRender::renderTexture(pX + Player[A].Location.Width / 2.0 - 16, pY - 30, 32, 32, GFX.Boot[Player[A].MountType], 0, 32 * Player[A].MountFrame, c, c, c);

                    if(Player[A].MountType == 3)
                    {
                        Player[A].YoshiWingsFrameCount += 1;
                        Player[A].YoshiWingsFrame = 0;
                        if(Player[A].YoshiWingsFrameCount <= 12)
                            Player[A].YoshiWingsFrame = 1;
                        else if(Player[A].YoshiWingsFrameCount >= 24)
                            Player[A].YoshiWingsFrameCount = 0;
                        XRender::renderTexture(pX + Player[A].Location.Width / 2.0 - 16 + 20, pY - 30 - 10, 32, 32, GFX.YoshiWings, 0, 0 + 32 * Player[A].YoshiWingsFrame, c, c, c);
                    }
                }
            }
            else if(Player[A].Character == 5)
            {
                XRender::renderTexture(pX + LinkFrameX[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                      pY - Player[A].Location.Height + LinkFrameY[(Player[A].State * 100) + (Player[A].Frame * Player[A].Direction)],
                                      100, 100,
                                      GFXLinkBMP[Player[A].State],
                                      pfrX(Player[A].Frame * Player[A].Direction),
                                      pfrY(Player[A].Frame * Player[A].Direction),
                                      c, c, c);
            }
        }
        A = numPlayers + 1;
        pX = margin * 1.5 - 64 + 48 * A;
        // Print lives on the screen
        XRender::renderTexture(pX, marginTop - 4 - GFX.Interface[3].h, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
        XRender::renderTexture(pX + 40, marginTop - 2 - GFX.Interface[3].h, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

        SuperPrint(std::to_string(int(Lives)), 1, pX + 62, marginTop-18);
        // Print coins on the screen
        auto& coin_icon = (Player[1].Character == 5) ? GFX.Interface[6] : GFX.Interface[2];
        XRender::renderTexture(pX + 16, marginTop - 42, coin_icon.w, coin_icon.h, coin_icon, 0, 0);

        XRender::renderTexture(pX + 40, marginTop - 40, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

        SuperPrint(std::to_string(Coins), 1, pX + 62, marginTop - 40);
        // Print stars on the screen
        if(numStars > 0)
        {
            XRender::renderTexture(pX + 16, marginTop - 64, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
            XRender::renderTexture(pX + 40, marginTop - 62, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
            SuperPrint(std::to_string(numStars), 1, pX + 62, marginTop - 62);
        }

        // Print the level's name
        if(!WorldPlayer[1].LevelName.empty())
        {
            s_DrawLevelName(WorldPlayer[1].LevelName, pX + 116, marginTop - 21, sW - margin - (pX + 116));
        }

        XRender::setViewport(0, 0, ScreenW, ScreenH);

        g_worldScreenFader.draw();

        if(PrintFPS > 0)
            SuperPrint(std::to_string(int(PrintFPS)), 1, 8, 8, 0.f, 1.f, 0.f);

        g_stats.print();

        if(!BattleMode && !GameMenu && g_config.show_episode_title)
        {
            int y;
            float alpha;

            // big screen, display at top
            if(ScreenH >= 640)
            {
                y = 20;
                switch(g_config.show_episode_title)
                {
                    case Config_t::EPISODE_TITLE_ON:
                    case Config_t::EPISODE_TITLE_AUTO:
                    case Config_t::EPISODE_TITLE_ON_ALWAYS:
                        alpha = 1.0f;
                        break;
                    case Config_t::EPISODE_TITLE_TRANSPARENT:
                    case Config_t::EPISODE_TITLE_TRANSPARENT_ALWAYS:
                        alpha = 0.5f;
                        break;
                    default:
                        alpha = 0.f;
                }
            }
            // small screen, maybe don't display, display at bottom
            else
            {
                y = ScreenH - 60;
                switch(g_config.show_episode_title)
                {
                    case Config_t::EPISODE_TITLE_ON_ALWAYS:
                        alpha = 1.0f;
                        break;
                    case Config_t::EPISODE_TITLE_AUTO:
                    case Config_t::EPISODE_TITLE_TRANSPARENT_ALWAYS:
                        alpha = 0.5f;
                        break;
                    case Config_t::EPISODE_TITLE_TRANSPARENT:
                    case Config_t::EPISODE_TITLE_ON:
                    default:
                        alpha = 0.f;
                }
            }

            SuperPrintScreenCenter(WorldName, 3, y, 1.f, 1.f, 1.f, alpha);
        }

        speedRun_renderControls(1, -1, SPEEDRUN_ALIGN_LEFT);

        speedRun_renderTimer();
    }

    XRender::setViewport(0, 0, ScreenW, ScreenH);

    // this code is for both non-editor and editor cases
    {
        // render special screens
        if(GamePaused == PauseCode::PauseScreen)
            PauseScreen::Render();

        if(QuickReconnectScreen::g_active)
            QuickReconnectScreen::Render();

        if(GamePaused == PauseCode::Reconnect || GamePaused == PauseCode::DropAdd)
        {
            ConnectScreen::Render();
            XRender::renderTexture(int(SharedCursor.X), int(SharedCursor.Y), GFX.ECursor[2]);
        }

        if(GamePaused == PauseCode::TextEntry)
            TextEntryScreen::Render();

        if(!skipRepaint)
            XRender::repaint();

        XRender::setTargetScreen();
    }

    if(TakeScreen)
        ScreenShot();

    frameRenderEnd();

//    if(XRender::lazyLoadedBytes() > 200000) // Reset timer while loading many pictures at the same time
//        resetFrameTimer();
    XRender::lazyLoadedBytesReset();
}
