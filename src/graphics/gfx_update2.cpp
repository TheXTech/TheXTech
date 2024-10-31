/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <fontman/font_manager.h>

#include "../globals.h"
#include "../gfx.h"
#include "../frame_timer.h"
#include "../graphics.h"
#include "../collision.h"
#include "../player.h"
#include "../config.h"
#include "../sound.h"
#include "../main/speedrunner.h"
#include "../main/trees.h"
#include "../main/screen_pause.h"
#include "../main/screen_connect.h"
#include "../main/screen_options.h"
#include "../main/screen_quickreconnect.h"
#include "../main/screen_textentry.h"
#include "../game_main.h"
#include "../main/world_globals.h"
#include "main/level_medals.h"
#include "../core/render.h"
#include "../screen_fader.h"

#include "graphics/gfx_frame.h"
#include "graphics/gfx_marquee.h"
#include "graphics/gfx_world.h"

#include "gfx_special_frames.h"

#include "draw_planes.h"

#include <fmt_format_ne.h>


//! extra non-gameplay related draws (menus and information display), defined in gfx_update.cpp
void UpdateGraphicsMeta();

static inline int computeStarsShowingPolicy(int ll, int cur)
{
    // Disable if not allowed globally
    if(!g_config.world_map_stars_show)
        return Config_t::MAP_STARS_HIDE;

    // Level individual
    if(ll > Config_t::MAP_STARS_UNSPECIFIED)
    {
        if(ll == Config_t::MAP_STARS_COLLECTED && cur <= 0)
            return Config_t::MAP_STARS_HIDE;
        return ll;
    }

    // World map-wide
    // (IMPORTANT NOTE: "hide" is now ignored as a map-wide setting, since the user has responsibility for setting this)
    if(WorldStarsShowPolicy > Config_t::MAP_STARS_HIDE)
    {
        if(WorldStarsShowPolicy == Config_t::MAP_STARS_COLLECTED && cur <= 0)
            return Config_t::MAP_STARS_HIDE;
        return WorldStarsShowPolicy;
    }

    // Default behavior if not disabled is collected only
    if(cur <= 0)
        return Config_t::MAP_STARS_HIDE;

    return Config_t::MAP_STARS_COLLECTED;
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

    if(g_config.enable_frameskip && frameSkipNeeded())
        return;

#ifdef __16M__
    if(!XRender::ready_for_frame())
    {
        if(g_config.unlimited_framerate)
            frameNextInc();
        return;
    }
#endif

    XRender::setTargetTexture();

    frameNextInc();
    frameRenderStart();

    g_stats.reset();

    // int A = 0;
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
        vScreen[Z].Width = Screens[0].W;
        vScreen[Z].Height = Screens[0].H;
    }
    else
    {
        SetupScreens(false);
        GetvScreenWorld(vScreen[Z]);

        if(qScreen)
        {
            qScreen = Update_qScreen(1, g_worldCamSpeed, g_worldCamSpeed);

            if(qScreen && g_worldPlayCamSound)
                PlaySound(SFX_Camera);

            // reset cam sound
            g_worldPlayCamSound = false;
        }
        // reset cam speed
        else
        {
            g_worldCamSpeed = 1.5;
        }
    }

    SpecialFrames();
    CommonFrames();
    WorldFrames();

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

    XRender::setDrawPlane(PLANE_GAME_BACKDROP);

    XRender::clearBuffer();
    XRender::resetViewport();
    DrawBackdrop(Screens[0]);

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

#ifdef __3DS__
    XRender::setTargetLayer(1);
#endif

    XRender::setViewport(vScreen[Z].TargetX() - XRender::TargetOverscanX, vScreen[Z].TargetY(), vScreen[Z].Width + 2 * XRender::TargetOverscanX, vScreen[Z].Height);

    int camX = vScreen[Z].CameraAddX() + XRender::TargetOverscanX;
    int camY = vScreen[Z].CameraAddY();

    double sLeft = -vScreen[1].X - 2 * XRender::TargetOverscanX;
    double sTop = -vScreen[1].Y;
    double sRight = -vScreen[1].X + vScreen[1].Width + 2 * XRender::TargetOverscanX;
    double sBottom = -vScreen[1].Y + vScreen[1].Height;

    {
        Location_t sView;
        sView.X = sLeft;
        sView.Y = sTop;
        sView.Width = sRight - sLeft;
        sView.Height = sBottom - sTop;

        XRender::setDrawPlane(PLANE_WLD_TIL);

        //for(A = 1; A <= numTiles; A++)
        for(Tile_t* t : treeWorldTileQuery(sLeft, sTop, sRight, sBottom, true))
        {
            Tile_t &tile = *t;
            SDL_assert(IF_INRANGE(tile.Type, 1, maxTileType));

            g_stats.checkedTiles++;
            if(CheckCollision(sView, tile.Location))
            {
                g_stats.renderedTiles++;
//                XRender::renderTexture(camX + Tile[A].Location.X, camY + Tile[A].Location.Y, Tile[A].Location.Width, Tile[A].Location.Height, GFXTile[Tile[A].Type], 0, TileHeight[Tile[A].Type] * TileFrame[Tile[A].Type]);
                XRender::renderTextureBasic(camX + tile.Location.X,
                                      camY + tile.Location.Y,
                                      tile.Location.Width,
                                      tile.Location.Height,
                                      GFXTileBMP[tile.Type], 0, TileHeight[tile.Type] * TileFrame[tile.Type]);
            }
        }

        XRender::setDrawPlane(PLANE_WLD_SCN);

        //for(A = 1; A <= numScenes; A++)
        for(Scene_t* t : treeWorldSceneQuery(sLeft, sTop, sRight, sBottom, true))
        {
            Scene_t &scene = *t;
            SDL_assert(IF_INRANGE(scene.Type, 1, maxSceneType));

            g_stats.checkedScenes++;
            if(CheckCollision(sView, scene.Location) && (WorldEditor || scene.Active))
            {
                g_stats.renderedScenes++;
//                XRender::renderTexture(camX + scene.Location.X, camY + scene.Location.Y, scene.Location.Width, scene.Location.Height, GFXSceneMask[scene.Type], 0, SceneHeight[scene.Type] * SceneFrame[scene.Type]);
//                XRender::renderTexture(camX + scene.Location.X, camY + scene.Location.Y, scene.Location.Width, scene.Location.Height, GFXScene[scene.Type], 0, SceneHeight[scene.Type] * SceneFrame[scene.Type]);
                XRender::renderTextureBasic(camX + scene.Location.X,
                                      camY + scene.Location.Y,
                                      scene.Location.Width, scene.Location.Height,
                                      GFXSceneBMP[scene.Type], 0, SceneHeight[scene.Type] * SceneFrame[scene.Type]);
            }
        }

        XRender::setDrawPlane(PLANE_WLD_PTH);

        //for(A = 1; A <= numWorldPaths; A++)
        for(WorldPath_t* t : treeWorldPathQuery(sLeft, sTop, sRight, sBottom, true))
        {
            WorldPath_t &path = *t;
            SDL_assert(IF_INRANGE(path.Type, 1, maxPathType));

            g_stats.checkedPaths++;
            if(CheckCollision(sView, path.Location) && (WorldEditor || path.Active))
            {
                g_stats.renderedPaths++;
//                XRender::renderTexture(camX + path.Location.X, camY + path.Location.Y, path.Location.Width, path.Location.Height, GFXPathMask[path.Type], 0, 0);
//                XRender::renderTexture(camX + path.Location.X, camY + path.Location.Y, path.Location.Width, path.Location.Height, GFXPath[path.Type], 0, 0);
                XRender::renderTextureBasic(camX + path.Location.X,
                                      camY + path.Location.Y,
                                      path.Location.Width, path.Location.Height,
                                      GFXPathBMP[path.Type], 0, 0);
            }
        }

        XRender::setDrawPlane(PLANE_WLD_LVL);

        //for(A = 1; A <= numWorldLevels; A++)
        for(WorldLevel_t* t : treeWorldLevelQuery(sLeft, sTop, sRight, sBottom, true))
        {
            WorldLevel_t &lvlP = *t;
            SDL_assert(IF_INRANGE(lvlP.Type, 0, maxLevelType));

            g_stats.checkedLevels++;

            // using this for the onscreen collision fixes bugs where levels with large graphics or Big Path backgrounds would not be drawn when partially offscreen
            //   could use a compat check here to choose between LocationOnscreen and Location if desired
            Location_t locOnscreen = lvlP.LocationOnscreen();

            if(CheckCollision(sView, locOnscreen) && (WorldEditor || lvlP.Active))
            {
                Location_t locGFX = lvlP.LocationGFX();

                g_stats.renderedLevels++;

                if(lvlP.Path)
                {
                    XRender::renderTextureBasic(camX + lvlP.Location.X,
                                          camY + lvlP.Location.Y,
                                          lvlP.Location.Width,
                                          lvlP.Location.Height,
                                          GFXLevelBMP[0], 0, 0);
                }

                if(lvlP.Path2)
                {
                    XRender::renderTextureBasic(camX + lvlP.Location.X - 16,
                                          camY + 8 + lvlP.Location.Y,
                                          64, 32,
                                          GFXLevelBMP[29], 0, 0);
                }

                XRender::renderTexture(camX + locGFX.X,
                                      camY + locGFX.Y,
                                      locGFX.Width, locGFX.Height,
                                      GFXLevelBMP[lvlP.Type], 0, 32 * LevelFrame[lvlP.Type]);
            }
        }
    }

    if(WorldEditor)
    {
        XRender::setDrawPlane(PLANE_WLD_EFF);

        for(int A = 1; A <= numEffects; A++)
        {
            if(vScreenCollision(Z, Effect[A].Location))
            {
                XRender::renderTexture(camX + Effect[A].Location.X,
                    camY + Effect[A].Location.Y,
                    Effect[A].Location.Width, Effect[A].Location.Height,
                    GFXEffect[Effect[A].Type], 0, Effect[A].Frame * EffectHeight[Effect[A].Type]);
            }
        }

        XRender::setDrawPlane(PLANE_WLD_INFO);

        for(WorldMusic_t* t : treeWorldMusicQuery(sLeft, sTop, sRight, sBottom, true))
        {
            WorldMusic_t &music = *t;
            if(vScreenCollision(Z, music.Location))
            {
                XRender::renderRect(camX + music.Location.X, camY + music.Location.Y, 32, 32,
                    {255, 0, 255}, false);
                SuperPrint(std::to_string(music.Type), 1, camX + music.Location.X + 2, camY + music.Location.Y + 2);
            }
        }

        for(int A = 1; A <= numWorldAreas; A++)
        {
            WorldArea_t &area = WorldArea[A];
            if(vScreenCollision(Z, static_cast<Location_t>(area.Location)))
            {
                // single color for now
                XTColor color = XTColorF(1.0f, 0.8f, 0.2f);

                // draw rect with outline
                XRender::renderRect(camX + area.Location.X, camY + area.Location.Y,
                    area.Location.Width, area.Location.Height,
                    {0, 0, 0}, false);
                XRender::renderRect(camX + area.Location.X + 1, camY + area.Location.Y + 1,
                    area.Location.Width - 2, area.Location.Height - 2,
                    {0, 0, 0}, false);
                XRender::renderRect(camX + area.Location.X + 2, camY + area.Location.Y + 2,
                    area.Location.Width - 4, area.Location.Height - 4,
                    color, false);
                XRender::renderRect(camX + area.Location.X + 3, camY + area.Location.Y + 3,
                    area.Location.Width - 6, area.Location.Height - 6,
                    color, false);

                // highlight selectable area
                XRender::renderRect(camX + area.Location.X + 4, camY + area.Location.Y + 4,
                    28, 28,
                    XTColorF(1.0f, 1.0f, 0.0f, 0.5f), true);

                // label with index
                SuperPrint(std::to_string(A), 1, camX + area.Location.X + 4, camY + area.Location.Y + 4);
            }
        }

#ifdef __3DS__
        XRender::setTargetLayer(3);
#endif

        DrawEditorWorld();
    }
    else
    { // NOT AN EDITOR!!!
        XRender::setDrawPlane(PLANE_WLD_PLR);

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

//        XRender::renderTexture(camX + WorldPlayer[1].Location.X, camY + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayerMask[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);
//        XRender::renderTexture(camX + WorldPlayer[1].Location.X, camY + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight, WorldPlayer[1].Location.Width, WPHeight, GFXPlayer[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);
        XRender::renderTexture(camX + WorldPlayer[1].Location.X,
                              camY + WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - WPHeight,
                              WorldPlayer[1].Location.Width, WPHeight,
                              GFXPlayerBMP[WorldPlayer[1].Type], 0, WPHeight * WorldPlayer[1].Frame);

        if(WorldPlayer[1].LevelIndex)
        {
            XRender::setDrawPlane(PLANE_WLD_INFO);

            auto &l = WorldLevel[WorldPlayer[1].LevelIndex];

            auto policy = computeStarsShowingPolicy(l.starsShowPolicy, l.curStars);

            int p_center_x = camX + WorldPlayer[1].Location.X + (WorldPlayer[1].Location.Width / 2);
            int info_y = camY + WorldPlayer[1].Location.Y - 32;

            if(l.save_info.inited() && l.save_info.max_stars > 0 && policy > Config_t::MAP_STARS_HIDE)
            {
                std::string label;

                if(policy >= Config_t::MAP_STARS_SHOW)
                    label = fmt::format_ne("{0}/{1}", l.curStars, l.save_info.max_stars);
                else
                    label = fmt::format_ne("{0}", l.curStars);

                int len = SuperTextPixLen(label, 3);
                int totalLen = len + GFX.Interface[1].w + GFX.Interface[5].w + 8 + 4;
                int x = p_center_x - (totalLen / 2);

                XRender::renderTexture(x, info_y, GFX.Interface[5]);
                XRender::renderTexture(x + GFX.Interface[5].w + 8, info_y, GFX.Interface[1]);
                SuperPrint(label, 3, x + GFX.Interface[1].w + GFX.Interface[5].w + 8 + 4, info_y);
                info_y -= 20;
            }

            if(l.save_info.inited() && l.save_info.max_medals > 0 && true)
            {
                uint8_t ckpt = (Checkpoint == FileNamePathWorld + l.FileName) ? g_curLevelMedals.checkpoint : 0;

                DrawMedals(p_center_x, info_y, true, l.save_info.max_medals, 0, ckpt, l.save_info.medals_got, l.save_info.medals_best);
            }
        }

        XRender::setViewport(Screens[0].TargetX(), Screens[0].TargetY(), Screens[0].W, Screens[0].H);

#ifdef __3DS__
        XRender::setTargetLayer(2);
#endif

        XRender::setDrawPlane(PLANE_WLD_FRAME);

//        XRender::renderTexture(0, 0, 800, 130, GFX.Interface[4], 0, 0);
        DrawWorldMapFrame(vScreen[Z]);


        int pX = vScreen[Z].ScreenLeft + 32 - 64 + 48;
        int pY = vScreen[Z].ScreenTop - 6;


        XRender::setDrawPlane(PLANE_WLD_HUD);

        // prepare for player draw
        vScreen[0].X = 0;
        vScreen[0].Y = 0;
        vScreen[0].Width = XRender::TargetW;
        vScreen[0].Height = XRender::TargetH;

        for(int A = 1; A <= numPlayers; A++)
        {
            DrawPlayerWorld(Player[A], pX, pY);
            pX += 48;
        }

        // Print lives on the screen
        DrawLives(pX + 32, vScreen[Z].ScreenTop - 20, Lives, g_100s);

        // Print coins on the screen
        auto& coin_icon = (Player[1].Character == 5) ? GFX.Interface[6] : GFX.Interface[2];

        XRender::renderTexture(pX + 16, vScreen[Z].ScreenTop - 42, coin_icon);
        XRender::renderTexture(pX + 40, vScreen[Z].ScreenTop - 40, GFX.Interface[1]);
        SuperPrint(std::to_string(Coins), 1, pX + 62, vScreen[Z].ScreenTop - 40);

        // Print stars on the screen
        if(numStars > 0)
        {
            XRender::renderTexture(pX + 16, vScreen[Z].ScreenTop - 64, GFX.Interface[5]);
            XRender::renderTexture(pX + 40, vScreen[Z].ScreenTop - 62, GFX.Interface[1]);
            SuperPrint(std::to_string(numStars), 1, pX + 62, vScreen[Z].ScreenTop - 62);
        }

        // Print the level's name
        if(WorldPlayer[1].LevelIndex)
        {
            int lnlx = pX + 116;
            int lnrx = vScreen[Z].ScreenLeft + vScreen[Z].Width;

            MarqueeSpec marquee_spec(lnrx - lnlx, 10, 64, 32, -1);
            static MarqueeState marquee_state;

            // could make these arrays if multiple world players ever supported
            static vbint_t cache_LevelIndex;
            static double cache_vScreen_W = 0.0;

            static std::string cache_LevelName_Split;
            static int cache_LevelName_H = 0;

            int font = FontManager::fontIdFromSmbxFont(2);

            if(cache_LevelIndex != WorldPlayer[1].LevelIndex || cache_vScreen_W != vScreen[Z].Width)
            {
                cache_LevelIndex = WorldPlayer[1].LevelIndex;
                cache_vScreen_W = vScreen[Z].Width;

                marquee_state.reset_width();

                int max_width = lnrx - lnlx;

                cache_LevelName_Split = WorldLevel[WorldPlayer[1].LevelIndex].LevelName;
                // mutates cache_LevelName_Split
                cache_LevelName_H = FontManager::optimizeTextPx(cache_LevelName_Split, max_width, font).h();
            }

            if(g_config.world_map_lvlname_marquee || cache_LevelName_H > vScreen[Z].ScreenTop - 21 - 8)
            {
                SuperPrintMarquee(WorldLevel[WorldPlayer[1].LevelIndex].LevelName, 2, lnlx, vScreen[Z].ScreenTop - 21, marquee_spec, marquee_state);
                marquee_state.advance(marquee_spec);
            }
            else
            {
                FontManager::printText(cache_LevelName_Split.c_str(), cache_LevelName_Split.size(),
                                        lnlx, vScreen[Z].ScreenTop - 21 - cache_LevelName_H,
                                        font);
            }
        }

#ifdef __3DS__
        if(GamePaused != PauseCode::Options)
            XRender::setTargetLayer(3);
#endif

        XRender::resetViewport();

        for(int plr_i = 0; plr_i < l_screen->player_count; plr_i++)
            speedRun_renderControls(plr_i, -1, SPEEDRUN_ALIGN_AUTO);
    }

    // draw on-screen menus and meta-information
    UpdateGraphicsMeta();

    if(!skipRepaint)
        XRender::repaint();

    frameRenderEnd();

//    if(XRender::lazyLoadedBytes() > 200000) // Reset timer while loading many pictures at the same time
//        resetFrameTimer();
    XRender::lazyLoadedBytesReset();
}
