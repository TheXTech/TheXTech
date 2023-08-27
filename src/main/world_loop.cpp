﻿/*
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

#include <Utils/files.h>
#include <Integrator/integrator.h>
#include <pge_delay.h>
#include <fmt_format_ne.h>
#include <sdl_proxy/sdl_stdinc.h>

#include "../globals.h"
#include "../frame_timer.h"
#include "../game_main.h"
#include "../sound.h"
#include "../controls.h"
#include "../graphics.h"
#include "../collision.h"
#include "../player.h"
#include "../main/trees.h"
#include "../core/events.h"
#include "../compat.h"
#include "../config.h"
#include "gfx.h"
#include "world_globals.h"
#include "level_file.h"
#include "speedrunner.h"
#include "screen_quickreconnect.h"
#include "screen_connect.h"
#include "main/game_strings.h"

#include "global_dirs.h"

//! Holds the screen overlay for the world map
ScreenFader g_worldScreenFader;

// world music index of the current world section
static int s_currentWorldSection = 0;
static bool s_playCamSound = false;
static int s_camMult = 1;
static double s_pathX = 0;
static double s_pathY = 0;


void worldWaitForFade(int waitTicks)
{
    bool ticks = waitTicks > 0;
    while(((!ticks && !g_worldScreenFader.isComplete()) || (ticks && waitTicks >= 0)) && GameIsActive)
    {
        XEvents::doEvents();

        if(canProceedFrame())
        {
            computeFrameTime1();
            UpdateGraphics2();
            UpdateSound();
            XEvents::doEvents();
            computeFrameTime2();
            g_worldScreenFader.update();
            if(waitTicks >= 0)
                waitTicks--;
        }

        if(!MaxFPS)
            PGE_Delay(1);
    }
}


bool worldHasFrameAssets()
{
    return GFX.WorldMapFrame_Tile.inited && (!GFX.Interface[4].inited || !GFX.isCustom(37) || GFX.isCustom(69));
}

static inline int computeStarsShowingPolicy(int ll, int cur)
{
    // Level individual
    if(ll > Compatibility_t::STARS_UNSPECIFIED)
    {
        if(ll == Compatibility_t::STARS_SHOW_COLLECTED_ONLY && cur <= 0)
            return Compatibility_t::STARS_DONT_SHOW;
        return ll;
    }

    // World map-wide
    if(WorldStarsShowPolicy > Compatibility_t::STARS_UNSPECIFIED)
    {
        if(WorldStarsShowPolicy == Compatibility_t::STARS_SHOW_COLLECTED_ONLY && cur <= 0)
            return Compatibility_t::STARS_DONT_SHOW;
        return WorldStarsShowPolicy;
    }

    // Compatibility settings
    if(g_compatibility.world_map_stars_show_policy > Compatibility_t::STARS_UNSPECIFIED)
    {
        if(g_compatibility.world_map_stars_show_policy == Compatibility_t::STARS_SHOW_COLLECTED_ONLY && cur <= 0)
            return Compatibility_t::STARS_DONT_SHOW;

        return g_compatibility.world_map_stars_show_policy;
    }

    // Gameplay settings
    if(g_config.WorldMapStarShowPolicyGlobal == Compatibility_t::STARS_SHOW_COLLECTED_ONLY && cur <= 0)
        return Compatibility_t::STARS_DONT_SHOW;

    return g_config.WorldMapStarShowPolicyGlobal;
}

bool g_isWorldMusicNotSame(WorldMusic_t &mus)
{
    bool ret = false;
    ret |= (curWorldMusic != mus.Type);
    ret |= (mus.Type == CustomWorldMusicId() && curWorldMusicFile != GetS(mus.MusicFile));
    return ret;
}

void g_playWorldMusic(WorldMusic_t &mus)
{
    curWorldMusicFile = GetS(mus.MusicFile);
    StartMusic(mus.Type);
}

// returns size of largest box centered around loc contained in s
static inline double s_worldSectionArea(WorldMusicRef_t s, const Location_t& loc)
{
    double cx = loc.X + loc.Width / 2;
    double cy = loc.Y + loc.Height / 2;

    double side_x = SDL_min(cx - s->Location.X, s->Location.X + s->Location.Width - cx);
    double side_y = SDL_min(cy - s->Location.Y, s->Location.Y + s->Location.Height - cy);

    return side_x * side_y;
}

static inline bool s_worldUpdateMusic(const Location_t &loc, bool section_only = false)
{
    bool ret = false;

    int best_section = 0;
    double best_section_area = 0;
    double best_wasted_area = 0;

    for(WorldMusicRef_t t : treeWorldMusicQuery(loc, false))
    {
        WorldMusic_t &mus = *t;
        if(CheckCollision(loc, mus.Location))
        {
            if(mus.Location.Width >= 64 && mus.Location.Height >= 64)
            {
                double section_area = s_worldSectionArea(t, loc);
                double wasted_area = mus.Location.Width * mus.Location.Height - section_area;

                if(section_area >= best_section_area && (section_area > best_section_area || wasted_area < best_wasted_area))
                {
                    best_section = (int)t;
                    best_section_area = section_area;
                    best_wasted_area = wasted_area;
                }

                if(mus.Type == 0)
                    continue;
            }

            if(!section_only && g_isWorldMusicNotSame(mus))
            {
                g_playWorldMusic(mus);
                ret = true;
            }
        }
    }

    if(best_section != s_currentWorldSection)
    {
        s_currentWorldSection = best_section;

        if(!qScreen)
        {
            qScreen = true;
            s_playCamSound = true;
            qScreenLoc[1] = vScreen[1];
        }
    }

    return ret;
}

static inline double getWPHeight()
{
    switch(WorldPlayer[1].Type)
    {
    case 3:
        return 44.0;
        break;
    case 4:
        return 40.0;
        break;
    default:
        return 32.0;
        break;
    }
}

//static inline double getWorldPlayerX()
//{
//    return WorldPlayer[1].Location.X;
//}

//static inline double getWorldPlayerY()
//{
//    return WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - getWPHeight();
//}

static inline double getWorldPlayerCenterX()
{
    return WorldPlayer[1].Location.X + WorldPlayer[1].Location.Width / 2;
}

static inline double getWorldPlayerCenterY()
{
    return WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - getWPHeight() / 2;
}

static void s_SetvScreenWorld(double fx, double fy)
{
    double margin = 66;
    double marginTop = 130;
    double marginBottom = 66;

    if(ScreenH < 400)
    {
        marginBottom = 24;
        marginTop = 72;
    }
    else if(ScreenH < 500)
    {
        marginBottom = 32;
        marginTop = 96;
    }

    if(ScreenW < 400)
        margin = 24;
    else if(ScreenW < 600)
        margin = 32;
    else if(ScreenW < 800)
        margin = 48;

    vScreen[1].Top = marginTop;
    vScreen[1].Height = ScreenH - marginBottom - marginTop;
    vScreen[1].Left = margin;
    vScreen[1].Width = ScreenW - (margin * 2);


    vScreen[1].X = -fx + vScreen[1].Width / 2.0;
    vScreen[1].Y = -fy + vScreen[1].Height / 2.0;

    bool allowExpandedFrame = worldHasFrameAssets() /* || g_gameInfo.interface4_stretch*/;
    if(s_currentWorldSection != 0 || !g_config.world_map_expand_view || !g_compatibility.allow_multires || !allowExpandedFrame)
    {
        const Location_t& sLoc = ((s_currentWorldSection != 0 && allowExpandedFrame)
            ? static_cast<Location_t>(WorldMusic[s_currentWorldSection].Location)
            : newLoc(fx - 334, fy - 202, 668, 404));

        if(sLoc.Width < vScreen[1].Width)
        {
            vScreen[1].Left += (vScreen[1].Width - sLoc.Width) / 2;
            vScreen[1].Width = sLoc.Width;
            vScreen[1].X = -sLoc.X;
        }
        else if(-vScreen[1].X < sLoc.X)
            vScreen[1].X = -sLoc.X;
        else if(-vScreen[1].X > sLoc.X + sLoc.Width - vScreen[1].Width)
            vScreen[1].X = -(sLoc.X + sLoc.Width - vScreen[1].Width);

        if(sLoc.Height < vScreen[1].Height)
        {
            vScreen[1].Top += (vScreen[1].Height - sLoc.Height) / 2;
            vScreen[1].Height = sLoc.Height;
            vScreen[1].Y = -sLoc.Y;
        }
        else if(-vScreen[1].Y < sLoc.Y)
            vScreen[1].Y = -sLoc.Y;
        else if(-vScreen[1].Y > sLoc.Y + sLoc.Height - vScreen[1].Height)
            vScreen[1].Y = -(sLoc.Y + sLoc.Height - vScreen[1].Height);
    }

    vScreen[1].ScreenTop = vScreen[1].Top;
    vScreen[1].ScreenLeft = vScreen[1].Left;
}

static void s_SetvScreenWorld(const Location_t& loc)
{
    s_SetvScreenWorld(loc.X + loc.Width / 2, loc.Y + loc.Height / 2);
}

void worldResetSection()
{
    s_worldUpdateMusic(WorldPlayer[1].Location);
    qScreen = false;
    s_SetvScreenWorld(WorldPlayer[1].Location.X + WorldPlayer[1].Location.Width / 2.0, WorldPlayer[1].Location.Y + WorldPlayer[1].Location.Height / 2.0);
}

void WorldLoop()
{
    bool musicReset = false;
    Location_t tempLocation;
    int A = 0;
    int B = 0;
    bool allowFastMove = (g_config.worldMapFastMove || g_config.worldMapFastMove) && g_speedRunnerMode < SPEEDRUN_MODE_2;

    if(SingleCoop > 0)
        SingleCoop = 1;

    s_SetvScreenWorld(WorldPlayer[1].Location.X + WorldPlayer[1].Location.Width / 2.0, WorldPlayer[1].Location.Y + WorldPlayer[1].Location.Height / 2.0);

    if(qScreen)
    {
        qScreen = Update_qScreen(1, 4 * s_camMult, 4 * s_camMult);

        if(qScreen && s_playCamSound)
            PlaySound(SFX_Camera);

        if(!qScreen)
            s_camMult = 1;

        s_playCamSound = false;
    }

    if(numPlayers > 2)
        numPlayers = 1;

    for(B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 2)
            Player[B].Mount = 0;
    }

    speedRun_tick();
    Integrator::sync();
    UpdateGraphics2();

    if(!Controls::Update())
    {
        QuickReconnectScreen::g_active = true;

        if(!g_config.NoPauseReconnect && g_compatibility.pause_on_disconnect)
            PauseGame(PauseCode::PauseScreen, 0);
    }

    if(QuickReconnectScreen::g_active)
        QuickReconnectScreen::Logic();

    UpdateSound();

    g_worldScreenFader.update();

    if(curWorldLevel > 0)
    {
        if(LevelBeatCode > 0)
        {
            s_worldUpdateMusic(WorldPlayer[1].Location);
            s_SetvScreenWorld(WorldPlayer[1].Location);
            qScreen = false;

            for(A = 1; A <= 4; A++)
            {
                if(WorldLevel[curWorldLevel].LevelExit[A] == LevelBeatCode || WorldLevel[curWorldLevel].LevelExit[A] == -1)
                {
                    auto &l = WorldLevel[curWorldLevel];
                    WorldPlayer[1].LevelName = l.LevelName;
                    auto &s = WorldPlayer[1].stars;
                    s.cur = l.curStars;
                    s.max = l.maxStars;
                    s.displayPolicy = computeStarsShowingPolicy(l.starsShowPolicy, s.cur);
                    LevelPath(l, A);
                }
            }

            SaveGame();
            LevelBeatCode = 0;
        }
        else if(LevelBeatCode == -1)
        {
            s_worldUpdateMusic(WorldPlayer[1].Location);
            s_SetvScreenWorld(WorldPlayer[1].Location);
            qScreen = false;

            //for(A = 1; A <= numWorldLevels; A++)
            for(WorldLevelRef_t t : treeWorldLevelQuery(WorldPlayer[1].Location, SORTMODE_NONE))
            {
                WorldLevel_t &l = *t;
                if(CheckCollision(WorldPlayer[1].Location, l.Location))
                {
                    curWorldLevel = t;
                    WorldPlayer[1].LevelName = l.LevelName;
                    auto &s = WorldPlayer[1].stars;
                    s.cur = l.curStars;
                    s.max = l.maxStars;
                    s.displayPolicy = computeStarsShowingPolicy(l.starsShowPolicy, s.cur);
                    break;
                }
            }

            if(curWorldLevel > 0)
                LevelPath(WorldLevel[curWorldLevel], 5);

            SaveGame();
            LevelBeatCode = 0;
        }
    }
    else
        LevelBeatCode = 0;

    for(A = 1; A <= numPlayers; A++)
    {
        Player[A].Bumped = false;
        Player[A].Bumped2 = 0;
        Player[A].CanFly = false;
        Player[A].CanFly2 = false;
        Player[A].Effect = 0;
        Player[A].Effect2 = 0;
        Player[A].FlyCount = 0;
        Player[A].TailCount = 0;
        Player[A].Stoned = false;
    }

    if(WorldPlayer[1].Move > 0)
    {
        WorldPlayer[1].Frame2 += 1;
        if(WorldPlayer[1].Frame2 >= 8)
        {
            WorldPlayer[1].Frame2 = 0;
            WorldPlayer[1].Frame += 1;
        }
        if(WorldPlayer[1].Move == 1)
        {
            if(WorldPlayer[1].Frame < 6)
                WorldPlayer[1].Frame = 7;
            if(WorldPlayer[1].Frame > 7)
                WorldPlayer[1].Frame = 6;
        }
        if(WorldPlayer[1].Move == 4)
        {
            if(WorldPlayer[1].Frame < 2)
                WorldPlayer[1].Frame = 3;
            if(WorldPlayer[1].Frame > 3)
                WorldPlayer[1].Frame = 2;
        }
        if(WorldPlayer[1].Move == 3)
        {
            if(WorldPlayer[1].Frame < 0)
                WorldPlayer[1].Frame = 1;
            if(WorldPlayer[1].Frame > 1)
                WorldPlayer[1].Frame = 0;
        }
        if(WorldPlayer[1].Move == 2)
        {
            if(WorldPlayer[1].Frame < 4)
                WorldPlayer[1].Frame = 5;
            if(WorldPlayer[1].Frame > 5)
                WorldPlayer[1].Frame = 4;
        }
    }


    if(WorldPlayer[1].Move == 0)
    {
        tempLocation = WorldPlayer[1].Location;
        tempLocation.Width -= 8;
        tempLocation.Height -= 8;
        tempLocation.X += 4;
        tempLocation.Y += 4;
        WorldPlayer[1].LevelName.clear();

        //for(A = 1; A <= numWorldLevels; A++)
        for(auto t : treeWorldLevelQuery(tempLocation, SORTMODE_Z))
        {
            WorldLevel_t &l = *t;
            if(CheckCollision(tempLocation, l.Location))
            {
                WorldPlayer[1].LevelName = l.LevelName;
                auto &s = WorldPlayer[1].stars;
                s.cur = l.curStars;
                s.max = l.maxStars;
                s.displayPolicy = computeStarsShowingPolicy(l.starsShowPolicy, s.cur);
                break;
            }
        }

        if(SharedControls.Pause)
        {
            PauseGame(PauseCode::PauseScreen, 0);
        }
        for(int i = 1; i <= numPlayers; i++)
        {
            if(Player[i].Controls.Start && Player[i].UnStart)
                PauseGame(PauseCode::PauseScreen, 0);
            // only allow P1 to pause if multiplayer pause controls disabled
            if(!g_compatibility.multiplayer_pause_controls)
                break;
        }

        if(Player[1].Controls.Up)
        {
            tempLocation.Y -= 32;

            //for(A = 1; A <= numWorldPaths; A++)
            for(auto t : treeWorldPathQuery(tempLocation, SORTMODE_ID))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 1;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
            {
                WorldLevel_t &lvl = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, lvl.Location) && lvl.Active)
                    {
                        WorldPlayer[1].Move = 1;
                        break;
                    }
                }
            }
            if(WalkAnywhere)
                WorldPlayer[1].Move = 1;
            if(WorldPlayer[1].Move == 0)
            {
                WorldPlayer[1].Move3 = false;
                PlaySound(SFX_BlockHit);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Left)
        {
            tempLocation.X -= 32;

            //for(A = 1; A <= numWorldPaths; A++)
            for(auto t : treeWorldPathQuery(tempLocation, SORTMODE_ID))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 2;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
            {
                WorldLevel_t &lvl = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, lvl.Location) && lvl.Active)
                    {
                        WorldPlayer[1].Move = 2;
                        break;
                    }
                }
            }

            if(WalkAnywhere)
                WorldPlayer[1].Move = 2;
            if(WorldPlayer[1].Move == 0)
            {
                WorldPlayer[1].Move3 = false;
                PlaySound(SFX_BlockHit);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Down)
        {
            tempLocation.Y += 32;

            //for(A = 1; A <= numWorldPaths; A++)
            for(auto t : treeWorldPathQuery(tempLocation, SORTMODE_ID))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 3;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
            {
                WorldLevel_t &lvl = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, lvl.Location) && lvl.Active)
                    {
                        WorldPlayer[1].Move = 3;
                        break;
                    }
                }
            }

            if(WalkAnywhere)
                WorldPlayer[1].Move = 3;
            if(WorldPlayer[1].Move == 0)
            {
                WorldPlayer[1].Move3 = false;
                PlaySound(SFX_BlockHit);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Right)
        {
            tempLocation.X += 32;

            //for(A = 1; A <= numWorldPaths; A++)
            for(auto t : treeWorldPathQuery(tempLocation, SORTMODE_ID))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 4;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
            {
                WorldLevel_t &lvl = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, lvl.Location) && lvl.Active)
                    {
                        WorldPlayer[1].Move = 4;
                        break;
                    }
                }
            }

            if(WalkAnywhere)
                WorldPlayer[1].Move = 4;
            if(WorldPlayer[1].Move == 0)
            {
                WorldPlayer[1].Move3 = false;
                PlaySound(SFX_BlockHit);
                SoundPause[3] = 2;
            }
        }
        else if(Player[1].Controls.Jump && Player[1].UnStart)
        {
            //for(A = 1; A <= numWorldLevels; A++)
            for(WorldLevelRef_t t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
            {
                WorldLevel_t &lvl = *t;
                if(CheckCollision(tempLocation, lvl.Location))
                {
#if 0 // Moved into the handler of level ending
                    if(int(lvl.WarpX) != -1)
                        WorldPlayer[1].Location.X = lvl.WarpX;

                    if(int(lvl.WarpY) != -1)
                        WorldPlayer[1].Location.Y = lvl.WarpY;

                    if(int(lvl.WarpY) != -1 || int(lvl.WarpX) != -1)
                    {
                        LevelBeatCode = 6;
                        //for(B = 1; B <= numWorldLevels; B++)
                        for(auto t2 : treeWorldLevelQuery(WorldPlayer[1].Location, SORTMODE_ID))
                        {
                            WorldLevel_t &level2 = *t2;
                            if(CheckCollision(WorldPlayer[1].Location, level2.Location))
                            {
                                level2.Active = true;
                                curWorldLevel = t2;
                            }
                        }
                    }
#endif

                    if(!lvl.FileName.empty() && lvl.FileName != ".lvl" && lvl.FileName != ".lvlx")
                    {
                        addMissingLvlSuffix(lvl.FileName);
                        std::string levelPath = g_dirEpisode.resolveFileCaseExistsAbs(lvl.FileName);

                        if(!levelPath.empty())
                        {
                            // save which characters were present at level start
                            if(SwapCharAllowed())
                            {
                                pLogDebug("Save drop/add characters configuration at WorldLoop()");
                                ConnectScreen::SaveChars();
                            }

                            StartWarp = lvl.StartWarp;
                            StopMusic();
                            PlaySound(SFX_LevelSelect);
                            g_worldScreenFader.setupFader(2, 0, 65, ScreenFader::S_RECT,
                                                          true,
                                                          getWorldPlayerCenterX(), getWorldPlayerCenterY(), 1);
                            worldWaitForFade();

                            SoundPause[26] = 200;
                            curWorldLevel = t;
                            LevelSelect = false;

                            delayedMusicReset(); // Reset delayed music to prevent unexpected behaviour at loaded level

                            ClearLevel();

                            if(!OpenLevel(levelPath))
                            {
                                delayedMusicStart(); // Allow music being started
                                MessageText = fmt::format_ne(g_gameStrings.errorOpenFileFailed, lvl.FileName);
                                PauseGame(PauseCode::Message);
                                ErrorQuit = true;
                            }

                            GameThing(1000, 3);

                            break;
                        }
                        else
                        {
                            pLogWarning("Level file name \"%s\" at %d x %d (id=%d) was not found (directory %s)",
                                        lvl.FileName.c_str(),
                                        (int)lvl.Location.X,
                                        (int)lvl.Location.Y,
                                        lvl.Type,
                                        g_dirEpisode.getCurDir().c_str()
                            );
                        }
                    }
                    else if(int(lvl.WarpX) != -1 || int(lvl.WarpY) != -1)
                    {
                        musicReset = true;
                        StopMusic();
                        PlaySound(SFX_Warp);
//                        frmMain.setTargetTexture();
//                        frmMain.clearBuffer();
//                        frmMain.repaint();
//                        DoEvents();
//                        PGE_Delay(1000);
                        g_worldScreenFader.setupFader(3, 0, 65, ScreenFader::S_RECT,
                                                      true,
                                                      getWorldPlayerCenterX(), getWorldPlayerCenterY(), 1);
                        worldWaitForFade(65);

                        // Moved from above
                        if(int(lvl.WarpX) != -1)
                            WorldPlayer[1].Location.X = lvl.WarpX;
                        if(int(lvl.WarpY) != -1)
                            WorldPlayer[1].Location.Y = lvl.WarpY;

                        LevelBeatCode = 6;

                        //for(B = 1; B <= numWorldLevels; B++)
                        for(WorldLevelRef_t t2 : treeWorldLevelQuery(WorldPlayer[1].Location, SORTMODE_ID))
                        {
                            WorldLevel_t &level2 = *t2;
                            if(CheckCollision(WorldPlayer[1].Location, level2.Location))
                            {
                                level2.Active = true;
                                curWorldLevel = t2;
                            }
                        }
                        // -----------------------

                        g_worldScreenFader.setupFader(3, 65, 0, ScreenFader::S_RECT,
                                                      true,
                                                      getWorldPlayerCenterX(), getWorldPlayerCenterY(), 1);
//                        resetFrameTimer();
                    }
                }
            }
        }
        // else

        if(WorldPlayer[1].Move3 && WorldPlayer[1].Move == 0)
        {
            if(allowFastMove)
                PlayerPath(WorldPlayer[1]);

            if(WorldPlayer[1].Move == 0)
            {
                WorldPlayer[1].Move3 = false;
                PlaySound(SFX_Slide);
            }
        }

        if(WorldPlayer[1].Move == 0)
        {
            if(WorldPlayer[1].Frame == 5)
                WorldPlayer[1].Frame = 4;
            if(WorldPlayer[1].Frame == 3)
                WorldPlayer[1].Frame = 2;
        }

//        if(WorldPlayer[1].Move3 && WorldPlayer[1].Move == 0)
//        {
//            WorldPlayer[1].Move3 = false;
//            PlaySound(SFX_Slide);
//        }
        WorldPlayer[1].LastMove = 0;

        if(s_worldUpdateMusic(WorldPlayer[1].Location))
            musicReset = false;

        if(musicReset) // Resume the last playing music after teleportation
        {
            StartMusic(curWorldMusic);
            // musicReset = false;
        }

        if(delayMusicIsSet())
            delayedMusicStart();
    }
    else if(WorldPlayer[1].Move == 1)
    {
        WorldPlayer[1].Move2 += 2;
        WorldPlayer[1].Location.Y -= 2;

        if(WalkAnywhere || allowFastMove)
        {
            WorldPlayer[1].Move2 += 2;
            WorldPlayer[1].Location.Y -= 2;
        }

        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].LastMove = WorldPlayer[1].Move;
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 2)
    {
        WorldPlayer[1].Move2 += 2;
        WorldPlayer[1].Location.X -= 2;

        if(WalkAnywhere || allowFastMove)
        {
            WorldPlayer[1].Move2 += 2;
            WorldPlayer[1].Location.X -= 2;
        }

        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].LastMove = WorldPlayer[1].Move;
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 3)
    {
        WorldPlayer[1].Move2 += 2;
        WorldPlayer[1].Location.Y += 2;

        if(WalkAnywhere || allowFastMove)
        {
            WorldPlayer[1].Move2 += 2;
            WorldPlayer[1].Location.Y += 2;
        }

        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].LastMove = WorldPlayer[1].Move;
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 4)
    {
        WorldPlayer[1].Move2 += 2;
        WorldPlayer[1].Location.X += 2;

        if(WalkAnywhere || allowFastMove)
        {
            WorldPlayer[1].Move2 += 2;
            WorldPlayer[1].Location.X += 2;
        }

        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].LastMove = WorldPlayer[1].Move;
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
}

void LevelPath(const WorldLevel_t &Lvl, int Direction, bool Skp)
{
    Location_t tempLocation;
//    int A = 0;

    // Up
    if(Direction == 1 || Direction == 5)
    {
        tempLocation = static_cast<Location_t>(Lvl.Location);
        tempLocation.X +=  4;
        tempLocation.Y +=  4;
        tempLocation.Width -= 8;
        tempLocation.Height -=  8;
        tempLocation.Y -= 32;

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto t : treeWorldPathQuery(tempLocation, SORTMODE_NONE))
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                    s_camMult = 3;
                }
            }
        }
    }

    // Left
    if(Direction == 2 || Direction == 5)
    {
        tempLocation = static_cast<Location_t>(Lvl.Location);
        tempLocation.X += 4;
        tempLocation.Y += 4;
        tempLocation.Width -= 8;
        tempLocation.Height -= 8;
        tempLocation.X -= 32;

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto t : treeWorldPathQuery(tempLocation, SORTMODE_NONE))
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                    s_camMult = 3;
                }
            }
        }
    }

    // Down
    if(Direction == 3 || Direction == 5)
    {
        tempLocation = static_cast<Location_t>(Lvl.Location);
        tempLocation.X += 4;
        tempLocation.Y += 4;
        tempLocation.Width -= 8;
        tempLocation.Height -= 8;
        tempLocation.Y += 32;

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto t : treeWorldPathQuery(tempLocation, SORTMODE_NONE))
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                    s_camMult = 3;
                }
            }
        }
    }

    // Right
    if(Direction == 4 || Direction == 5)
    {
        tempLocation = static_cast<Location_t>(Lvl.Location);
        tempLocation.X += 4;
        tempLocation.Y += 4;
        tempLocation.Width -= 8;
        tempLocation.Height -= 8;
        tempLocation.X += 32;

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto t : treeWorldPathQuery(tempLocation, SORTMODE_NONE))
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                    s_camMult = 3;
                }
            }
        }
    }

    qScreen = true;
    qScreenLoc[1] = vScreen[1];
}

void PlayerPath(WorldPlayer_t &p)
{
    if(!p.LevelName.empty())
        return;

    Location_t tempLocation = p.Location;

    tempLocation.X += 4;
    tempLocation.Y += 4;
    tempLocation.Width -= 8;
    tempLocation.Height -= 8;

    // stop at level
    for(auto t : treeWorldLevelQuery(tempLocation, false))
    {
        WorldLevel_t& lvl = *t;
        if(CheckCollision(tempLocation, lvl.Location) && lvl.Active)
        {
            p.Move = 0;
            return;
        }
    }

    // stop at branch point
    int n_moves = 0;
    for(int B = 1; B <= 4; B++)
    {
        if(B == 1)
            tempLocation.Y -= 32; // Up
        else if(B == 2)
        {
            tempLocation.Y += 32; // Down
            tempLocation.X -= 32; // Left
        }
        else if(B == 3)
        {
            tempLocation.X += 32; // Right
            tempLocation.Y += 32; // Down
        }
        else if(B == 4)
        {
            tempLocation.Y -= 32; // Up
            tempLocation.X += 32; // Right
        }

        if(B != p.LastMove && B % 2 == p.LastMove % 2)
            continue;

        for(auto t : treeWorldPathQuery(tempLocation, false))
        {
            WorldPath_t& path = *t;
            if(CheckCollision(tempLocation, path.Location) && path.Active)
            {
                p.Move = B;
                n_moves ++;
                break;
            }
        }

        if(p.Move == B)
            continue;

        for(auto t : treeWorldLevelQuery(tempLocation, false))
        {
            WorldLevel_t& lvl = *t;
            if(CheckCollision(tempLocation, lvl.Location) && lvl.Active)
            {
                p.Move = B;
                n_moves ++;
                break;
            }
        }
    }

    if(n_moves > 1)
        p.Move = 0;
}

void PathPath(WorldPath_t &Pth, bool Skp)
{
    //int A = 0;
    int B = 0;

    Location_t tempLocation;
    tempLocation = static_cast<Location_t>(Pth.Location);
    tempLocation.X += 4;
    tempLocation.Y += 4;
    tempLocation.Width -= 8;
    tempLocation.Height -= 8;

    //for(A = 1; A <= numScenes; A++)
    for(auto t : treeWorldSceneQuery(tempLocation, SORTMODE_ID))
    {
        Scene_t &scene = *t;
        if(scene.Active)
        {
            if(CheckCollision(tempLocation, scene.Location))
                scene.Active = false;
        }
    }

    if(!Pth.Active && !Skp)
    {
        Pth.Active = true;

        s_worldUpdateMusic(static_cast<Location_t>(Pth.Location), true);

        s_playCamSound = false;

        if(g_compatibility.modern_section_change)
        {
            qScreen = true;
            qScreenLoc[1] = vScreen[1];
        }
        else
        {
            qScreen = false;
        }

        s_pathX = Pth.Location.X + Pth.Location.Width / 2.0;
        s_pathY = Pth.Location.Y + Pth.Location.Height / 2.0;

        PlaySound(SFX_NewPath);
        PathWait();
    }

    Pth.Active = true;

    for(B = 1; B <= 4; B++)
    {
        if(B == 1)
            tempLocation.Y -= 32; // Up
        else if(B == 2)
        {
            tempLocation.Y += 32; // Down
            tempLocation.X -= 32; // Left
        }
        else if(B == 3)
        {
            tempLocation.X += 32; // Right
            tempLocation.Y += 32; // Down
        }
        else if(B == 4)
        {
            tempLocation.Y -= 32; // Up
            tempLocation.X += 32; // Right
        }

        //for(A = 1; A <= numWorldPaths; A++)
        WorldPath_t* found = nullptr;
        for(WorldPath_t *path : treeWorldPathQuery(tempLocation, SORTMODE_ID))
        {
            D_pLogDebug("Found path activity: %d", (int)path->Active);
            if(!path->Active)
            {
                D_pLogDebugNA("Collision with path...");
                if(CheckCollision(tempLocation, path->Location))
                {
                    D_pLogDebugNA("Collision with path FOUND...");
                    found = path;
                    break;
                }
            }
        }
        if(found)
            PathPath(*found, Skp);

        //for(A = 1; A <= numWorldLevels; A++)
        for(auto t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
        {
            WorldLevel_t &lev = *t;
            D_pLogDebug("Found level activity: %d", (int)lev.Active);
            if(!lev.Active)
            {
                D_pLogDebugNA("Collision with level...");
                if(CheckCollision(tempLocation, lev.Location))
                {
                    D_pLogDebugNA("Collision with level FOUND...");
                    lev.Active = true;
                    if(!Skp)
                    {
                        s_worldUpdateMusic(static_cast<Location_t>(lev.Location), true);

                        s_playCamSound = false;

                        if(g_compatibility.modern_section_change)
                        {
                            qScreen = true;
                            qScreenLoc[1] = vScreen[1];
                        }
                        else
                        {
                            qScreen = false;
                        }

                        s_pathX = lev.Location.X + lev.Location.Width / 2.0;
                        s_pathY = lev.Location.Y + lev.Location.Height / 2.0;

                        PlaySound(SFX_NewPath);
                        PathWait();
                    }
                }
            }
        }
    }
}

void PathWait()
{
    int C = 0;

    resetFrameTimer();

    do
    {
        XEvents::doEvents();
        if(canProceedFrame())
        {
            speedRun_tick();

            s_SetvScreenWorld(s_pathX, s_pathY);

            if(qScreen)
            {
                D_pLogDebug("qScreen multiplier %d", s_camMult);
                qScreen = Update_qScreen(1, 2 * s_camMult, 2 * s_camMult);

                if(!qScreen)
                    s_camMult = 1;
            }

            UpdateGraphics2();
            UpdateSound();
            g_worldScreenFader.update();

            if(delayMusicIsSet())
                delayedMusicStart();

            C++;
            computeFrameTime1();
            XEvents::doEvents();
            computeFrameTime2();
        }

        if(!MaxFPS)
            PGE_Delay(1);
    } while(C < 24);

    resetFrameTimer();
}
