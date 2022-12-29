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

#include <fmt_format_ne.h>


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

    vScreen[Z].Left = 0;
    vScreen[Z].Top = 0;
    vScreen[Z].Width = ScreenW;
    vScreen[Z].Height = ScreenH;
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
    double sLeft, sTop, sRight, sBottom;
    {
        if(WorldEditor)
        {
            sLeft = -vScreenX[1];
            sTop = -vScreenY[1];
            sRight = -vScreenX[1] + vScreen[1].Width;
            sBottom = -vScreenY[1] + vScreen[1].Height;
        }
        else
        {
            sLeft = -vScreenX[1] + 64;
            sTop = -vScreenY[1] + 96;
            sRight = -vScreenX[1] + vScreen[1].Width - 64;
            sBottom = -vScreenY[1] + vScreen[1].Height - 64;
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

//        XRender::renderTexture(vScreenX[Z] + WorldPlayer[1].Location.X, vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayerMask[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);
//        XRender::renderTexture(vScreenX[Z] + WorldPlayer[1].Location.X, vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayer[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);
        XRender::renderTexture(vScreenX[Z] + WorldPlayer[1].Location.X,
                              vScreenY[Z] + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight,
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
                 int x = vScreenX[Z] + WorldPlayer[1].Location.X + (WorldPlayer[1].Location.Width / 2) - (totalLen / 2);
                 int y = vScreenY[Z] + WorldPlayer[1].Location.Y - 32;

                 XRender::renderTexture(x, y,
                                       GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
                 XRender::renderTexture(x + GFX.Interface[5].w + 8, y,
                                       GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
                 SuperPrint(label, 3, x + GFX.Interface[1].w + GFX.Interface[5].w + 8 + 4, y);
             }
        }

#ifdef __3DS__
        XRender::setTargetLayer(2);
#endif

//        XRender::renderTexture(0, 0, 800, 130, GFX.Interface[4], 0, 0);

        XRender::renderTexture(0, 0, 800, 130, GFX.Interface[4], 0, 0);
        XRender::renderTexture(0, 534, 800, 66, GFX.Interface[4], 0, 534);
        XRender::renderTexture(0, 130, 66, 404, GFX.Interface[4], 0, 130);
        XRender::renderTexture(734, 130, 66, 404, GFX.Interface[4], 734, 130);

#ifdef __3DS__
        XRender::setTargetLayer(3);
#endif

        for(A = 1; A <= numPlayers; A++)
        {
            int pX = 32 + 48 * A;
            int pY = 124;

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
            p.Location.X = pX - vScreenX[1];
            p.Location.Y = pY - vScreenY[1] - p.Location.Height;

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
        }

        A = numPlayers + 1;

        // Print lives on the screen
        XRender::renderTexture(32 + (48 * A), 126 - GFX.Interface[3].h, GFX.Interface[3].w, GFX.Interface[3].h, GFX.Interface[3], 0, 0);
        XRender::renderTexture(32 + (48 * A) + 40, 128 - GFX.Interface[3].h, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
        SuperPrint(std::to_string(int(Lives)), 1, 32 + (48 * A) + 62, 112);

        // Print coins on the screen
        if(Player[1].Character == 5)
            XRender::renderTexture(32 + (48 * A) + 16, 88, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[6], 0, 0);
        else
            XRender::renderTexture(32 + (48 * A) + 16, 88, GFX.Interface[2].w, GFX.Interface[2].h, GFX.Interface[2], 0, 0);
        XRender::renderTexture(32 + (48 * A) + 40, 90, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);

        SuperPrint(std::to_string(Coins), 1, 32 + (48 * A) + 62, 90);

        // Print stars on the screen
        if(numStars > 0)
        {
            XRender::renderTexture(32 + (48 * A) + 16, 66, GFX.Interface[5].w, GFX.Interface[5].h, GFX.Interface[5], 0, 0);
            XRender::renderTexture(32 + (48 * A) + 40, 68, GFX.Interface[1].w, GFX.Interface[1].h, GFX.Interface[1], 0, 0);
            SuperPrint(std::to_string(numStars), 1, 32 + (48 * A) + 62, 68);
        }

        // Print the level's name
        if(!WorldPlayer[1].LevelName.empty())
        {
            int lnlx = 32 + (48 * A) + 116;
            SuperPrint(WorldPlayer[1].LevelName, 2, lnlx, 109);
        }

        g_worldScreenFader.draw();

        if(PrintFPS > 0)
            SuperPrint(std::to_string(int(PrintFPS)), 1, 8, 8, 0.f, 1.f, 0.f);

        g_stats.print();

        if(!BattleMode && !GameMenu && g_config.show_episode_title)
        {
            int y = (ScreenH >= 640) ? 20 : ScreenH - 60;
            if(g_config.show_episode_title == Config_t::EPISODE_TITLE_TRANSPARENT)
                SuperPrintScreenCenter(WorldName, 3, y, 1.f, 1.f, 1.f, 0.5f);
            else
                SuperPrintScreenCenter(WorldName, 3, y, 1.f, 1.f, 1.f, 1.f);
        }

        speedRun_renderControls(1, -1);


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
