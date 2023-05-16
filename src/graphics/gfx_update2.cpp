/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <sdl_proxy/sdl_stdinc.h>

#include "../globals.h"
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

    cycleNextInc();

    if(FrameSkip && frameSkipNeeded())
        return;

    XRender::setTargetTexture();

    frameNextInc();
    frameRenderStart();

    g_stats.reset();

    int A = 0;
    // int B = 0;
    const int Z = 1;
    int WPHeight = 0;
//    Location_t tempLocation;
    //Z = 1;

    if(WorldEditor)
    {
        vScreen[Z].Left = 0;
        vScreen[Z].ScreenLeft = 0;
        vScreen[Z].Top = 0;
        vScreen[Z].ScreenTop = 0;
        vScreen[Z].Width = ScreenW;
        vScreen[Z].Height = ScreenH;
    }

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

#ifdef __3DS__
        XRender::setTargetLayer(0);
#endif

    XRender::clearBuffer();
    DrawBackdrop();

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

    XRender::setViewport(vScreen[Z].ScreenLeft, vScreen[Z].ScreenTop, vScreen[Z].Width, vScreen[Z].Height);

    double sLeft = -vScreen[1].X;
    double sTop = -vScreen[1].Y;
    double sRight = -vScreen[1].X + vScreen[1].Width;
    double sBottom = -vScreen[1].Y + vScreen[1].Height;

    {
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
//                XRender::renderTexture(vScreen[Z].X + Tile[A].Location.X, vScreen[Z].Y + Tile[A].Location.Y, Tile[A].Location.Width, Tile[A].Location.Height, GFXTile[Tile[A].Type], 0, TileHeight[Tile[A].Type] * TileFrame[Tile[A].Type]);
                XRender::renderTexture(vScreen[Z].X + tile.Location.X,
                                      vScreen[Z].Y + tile.Location.Y,
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
//                XRender::renderTexture(vScreen[Z].X + scene.Location.X, vScreen[Z].Y + scene.Location.Y, scene.Location.Width, scene.Location.Height, GFXSceneMask[scene.Type], 0, SceneHeight[scene.Type] * SceneFrame[scene.Type]);
//                XRender::renderTexture(vScreen[Z].X + scene.Location.X, vScreen[Z].Y + scene.Location.Y, scene.Location.Width, scene.Location.Height, GFXScene[scene.Type], 0, SceneHeight[scene.Type] * SceneFrame[scene.Type]);
                XRender::renderTexture(vScreen[Z].X + scene.Location.X,
                                      vScreen[Z].Y + scene.Location.Y,
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
//                XRender::renderTexture(vScreen[Z].X + path.Location.X, vScreen[Z].Y + path.Location.Y, path.Location.Width, path.Location.Height, GFXPathMask[path.Type], 0, 0);
//                XRender::renderTexture(vScreen[Z].X + path.Location.X, vScreen[Z].Y + path.Location.Y, path.Location.Width, path.Location.Height, GFXPath[path.Type], 0, 0);
                XRender::renderTexture(vScreen[Z].X + path.Location.X,
                                      vScreen[Z].Y + path.Location.Y,
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

            Location_t locGFX = level.LocationGFX();

            if(CheckCollision(sView, locGFX) && (WorldEditor || level.Active))
            {
                g_stats.renderedLevels++;

                if(level.Path)
                {
                    XRender::renderTexture(vScreen[Z].X + level.Location.X,
                                          vScreen[Z].Y + level.Location.Y,
                                          level.Location.Width,
                                          level.Location.Height,
                                          GFXLevelBMP[0], 0, 0);
                }

                if(level.Path2)
                {
                    XRender::renderTexture(vScreen[Z].X + level.Location.X - 16,
                                          vScreen[Z].Y + 8 + level.Location.Y,
                                          64, 32,
                                          GFXLevelBMP[29], 0, 0);
                }

                XRender::renderTexture(vScreen[Z].X + locGFX.X,
                                      vScreen[Z].Y + locGFX.Y,
                                      locGFX.Width, locGFX.Height,
                                      GFXLevelBMP[level.Type], 0, 32 * LevelFrame[level.Type]);
            }
        }
    }

    if(WorldEditor)
    {
        for(A = 1; A <= numEffects; A++)
        {
            if(vScreenCollision(Z, Effect[A].Location))
            {
                XRender::renderTexture(vScreen[Z].X + Effect[A].Location.X,
                    vScreen[Z].Y + Effect[A].Location.Y,
                    Effect[A].Location.Width, Effect[A].Location.Height,
                    GFXEffect[Effect[A].Type], 0, Effect[A].Frame * EffectHeight[Effect[A].Type]);
            }
        }

        for(WorldMusic_t* t : treeWorldMusicQuery(sLeft, sTop, sRight, sBottom, true))
        {
            WorldMusic_t &music = *t;
            if(vScreenCollision(Z, music.Location))
            {
                XRender::renderRect(vScreen[Z].X + music.Location.X, vScreen[Z].Y + music.Location.Y, 32, 32,
                    1.f, 0.f, 1.f, 1.f, false);
                SuperPrint(std::to_string(music.Type), 1, vScreen[Z].X + music.Location.X + 2, vScreen[Z].Y + music.Location.Y + 2);
            }
        }

#ifdef __3DS__
        XRender::setTargetLayer(3);
#endif

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

//        XRender::renderTexture(vScreen[Z].X + WorldPlayer[1].Location.X, vScreen[Z].Y + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayerMask[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);
//        XRender::renderTexture(vScreen[Z].X + WorldPlayer[1].Location.X, vScreen[Z].Y + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayer[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);
        XRender::renderTexture(vScreen[Z].X + WorldPlayer[1].Location.X,
                              vScreen[Z].Y + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight,
                              WorldPlayer[1].Location.Width, WPHeight,
                              GFXPlayerBMP[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);

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
                 int x = vScreen[Z].X + WorldPlayer[1].Location.X + (WorldPlayer[1].Location.Width / 2) - (totalLen / 2);
                 int y = vScreen[Z].Y + WorldPlayer[1].Location.Y - 32;

                 XRender::renderTexture(x, y,
                                       GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                 XRender::renderTexture(x + GFX.Interface[5].w + 8, y,
                                       GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                 SuperPrint(label, 3, x + GFX.Interface[1].w + GFX.Interface[5].w + 8 + 4, y);
             }
        }

        XRender::resetViewport();

#ifdef __3DS__
        XRender::setTargetLayer(2);
#endif

//        XRender::renderTexture(0, 0, 800, 130, GFX.Interface[4], 0, 0);
        if(worldHasFrameAssets())
        {
            bool border_valid = GFX.WorldMapFrame_Border.inited && (!GFX.isCustom(69) || GFX.isCustom(70));

            RenderFrame(newLoc(0, 0, ScreenW, ScreenH), newLoc(vScreen[Z].ScreenLeft, vScreen[Z].ScreenTop, vScreen[Z].Width, vScreen[Z].Height),
                GFX.WorldMapFrame_Tile, border_valid ? &GFX.WorldMapFrame_Border : nullptr, &g_worldMapFrameBorderInfo);

        }
        else
        {
            // render a legacy background, in MANY careful segments...
            constexpr bool do_stretch = true;

            // top-left
            XRender::renderTexture(vScreen[Z].ScreenLeft - 66, vScreen[Z].ScreenTop - 130, 66, 130, GFX.Interface[4], 0, 0);

            // top
            int orig_width = GFX.Interface[4].w - 66 - 66;
            if(do_stretch)
            {
                XRender::renderTextureScaleEx(vScreen[Z].ScreenLeft, vScreen[Z].ScreenTop - 130,
                    vScreen[Z].Width, 130,
                    GFX.Interface[4], 66, 0, orig_width, 130);
            }
            else
                for(int offset = 0; offset < vScreen[Z].Width; offset += orig_width)
            {
                XRender::renderTexture(vScreen[Z].ScreenLeft + offset, vScreen[Z].ScreenTop - 130,
                    SDL_min((int)vScreen[Z].Width - offset, orig_width), 130,
                    GFX.Interface[4], 66, 0);
            }

            // top-right
            XRender::renderTexture(vScreen[Z].ScreenLeft + vScreen[Z].Width, vScreen[Z].ScreenTop - 130, 66, 130 + 20, GFX.Interface[4], GFX.Interface[4].w - 66, 0);

            // left
            int orig_height = GFX.Interface[4].h - 130 - 66;
            if(do_stretch)
            {
                XRender::renderTextureScaleEx(vScreen[Z].ScreenLeft - 66, vScreen[Z].ScreenTop,
                    66, vScreen[Z].Height,
                    GFX.Interface[4], 0, 130, 66, orig_height);
            }
            else
                for(int offset = 0; offset < vScreen[Z].Height; offset += orig_height)
            {
                XRender::renderTexture(vScreen[Z].ScreenLeft - 66, vScreen[Z].ScreenTop + offset,
                    66, SDL_min((int)vScreen[Z].Height - offset, orig_height),
                    GFX.Interface[4], 0, 130);
            }

            // right
            orig_height = GFX.Interface[4].h - (130 + 20) - 66;
            if(do_stretch)
            {
                XRender::renderTextureScaleEx(vScreen[Z].ScreenLeft + vScreen[Z].Width, vScreen[Z].ScreenTop + 20,
                    66, vScreen[Z].Height - 20,
                    GFX.Interface[4], GFX.Interface[4].w - 66, 130 + 20, 66, orig_height);
            }
            else
                for(int offset = 20; offset < vScreen[Z].Height; offset += orig_height)
            {
                XRender::renderTexture(vScreen[Z].ScreenLeft + vScreen[Z].Width, vScreen[Z].ScreenTop + offset,
                    66, SDL_min((int)vScreen[Z].Height - offset, orig_height),
                    GFX.Interface[4], GFX.Interface[4].w - 66, 130 + 20);
            }

            // bottom-left
            XRender::renderTexture(vScreen[Z].ScreenLeft - 66, vScreen[Z].ScreenTop + vScreen[Z].Height, 66 + 34, 66, GFX.Interface[4], 0, GFX.Interface[4].h - 66);

            // bottom
            orig_width = GFX.Interface[4].w - (66 + 34) - 66;
            if(do_stretch)
            {
                XRender::renderTextureScaleEx(vScreen[Z].ScreenLeft + 34, vScreen[Z].ScreenTop + vScreen[Z].Height,
                    vScreen[Z].Width - 34, 66,
                    GFX.Interface[4], 100, GFX.Interface[4].h - 66, orig_width, 66);
            }
            else
                for(int offset = 34; offset < vScreen[Z].Width; offset += orig_width)
            {
                XRender::renderTexture(vScreen[Z].ScreenLeft + offset, vScreen[Z].ScreenTop + vScreen[Z].Height,
                    SDL_min((int)vScreen[Z].Width - offset, orig_width), 66,
                    GFX.Interface[4], 100, GFX.Interface[4].h - 66);
            }

            // bottom-right
            XRender::renderTexture(vScreen[Z].ScreenLeft + vScreen[Z].Width, vScreen[Z].ScreenTop + vScreen[Z].Height, 66, 66, GFX.Interface[4], GFX.Interface[4].w - 66, GFX.Interface[4].h - 66);
        }


        int pX = vScreen[Z].ScreenLeft + 32 - 64 + 48;
        int pY = vScreen[Z].ScreenTop - 6;

#ifdef __3DS__
        XRender::setTargetLayer(3);
#endif

        for(A = 1; A <= numPlayers; A++)
        {
            Player_t& p = Player[A];

            p.Direction = -1;
            p.Location.SpeedY = 0;
            p.Location.SpeedX = -1;
            p.Controls.Left = false;
            p.Controls.Right = false;
            p.SpinJump = false;
            p.Dead = false;
            p.Immune2 = false;
            p.Fairy = false;
            p.TimeToLive = 0;
            p.Effect = 0;
            p.MountSpecial = 0;
            p.HoldingNPC = 0;
            if(p.Duck)
                UnDuck(p);
            PlayerFrame(p);

            p.Location.Width = Physics.PlayerWidth[p.Character][p.State];
            p.Location.Height = Physics.PlayerHeight[p.Character][p.State];
            SizeCheck(p);
            p.Location.X = pX - vScreen[1].X;
            p.Location.Y = pY - vScreen[1].Y - p.Location.Height;

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

            DrawPlayer(p, 1);

            pX += 48;
        }

        A = numPlayers + 1;
        // Print lives on the screen
        XRender::renderTexture(pX, vScreen[Z].ScreenTop - 4 - GFX.Interface[3].h, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
        XRender::renderTexture(pX + 40, vScreen[Z].ScreenTop - 2 - GFX.Interface[3].h, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

        SuperPrint(std::to_string(int(Lives)), 1, pX + 62, vScreen[Z].ScreenTop - 18);
        // Print coins on the screen
        auto& coin_icon = (Player[1].Character == 5) ? GFX.Interface[6] : GFX.Interface[2];
        XRender::renderTexture(pX + 16, vScreen[Z].ScreenTop - 42, coin_icon.w, coin_icon.h, coin_icon, 0, 0);

        XRender::renderTexture(pX + 40, vScreen[Z].ScreenTop - 40, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

        SuperPrint(std::to_string(Coins), 1, pX + 62, vScreen[Z].ScreenTop - 40);
        // Print stars on the screen
        if(numStars > 0)
        {
            XRender::renderTexture(pX + 16, vScreen[Z].ScreenTop - 64, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
            XRender::renderTexture(pX + 40, vScreen[Z].ScreenTop - 62, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
            SuperPrint(std::to_string(numStars), 1, pX + 62, vScreen[Z].ScreenTop - 62);
        }

        // Print the level's name
        if(!WorldPlayer[1].LevelName.empty())
        {
            s_DrawLevelName(WorldPlayer[1].LevelName, pX + 116, vScreen[Z].ScreenTop - 21, vScreen[Z].ScreenLeft + vScreen[Z].Width - (pX + 116));
        }

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

        speedRun_renderControls(1, -1);
        if(numPlayers >= 2)
            speedRun_renderControls(2, -1);


        speedRun_renderTimer();
    }

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
