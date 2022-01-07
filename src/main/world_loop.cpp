/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <SDL2/SDL_timer.h>

#include <Utils/files.h>
#include <pge_delay.h>
#include <fmt_format_ne.h>

#include "../globals.h"
#include "../frame_timer.h"
#include "../game_main.h"
#include "../sound.h"
#include "../controls.h"
#include "../effect.h"
#include "../graphics.h"
#include "../collision.h"
#include "../main/trees.h"
#include "../core/events.h"
#include "../compat.h"
#include "../config.h"
#include "world_globals.h"
#include "level_file.h"
#include "speedrunner.h"
#include "screen_quickreconnect.h"

#include "../pseudo_vb.h"

//! Holds the screen overlay for the world map
ScreenFader g_worldScreenFader;

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

        PGE_Delay(1);
    }
}


static SDL_INLINE int computeStarsShowingPolicy(int ll, int cur)
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

static SDL_INLINE bool isWorldMusicNotSame(WorldMusic_t &mus)
{
    bool ret = false;
    ret |= (curWorldMusic != mus.Type);
    ret |= (mus.Type == CustomWorldMusicId() && curWorldMusicFile != mus.MusicFile);
    return ret;
}

static SDL_INLINE bool s_worldUpdateMusic(const Location_t &loc)
{
    bool ret = false;
    static WorldMusicPtrArr marr;

    if(marr.capacity() < 20)
        marr.reserve(20);

    treeWorldMusicQuery(loc, marr, false);

    for(auto *t : marr)
    {
        WorldMusic_t &mus = *t;
        if(CheckCollision(loc, mus.Location))
        {
            if(isWorldMusicNotSame(mus))
            {
                curWorldMusicFile = mus.MusicFile;
                StartMusic(mus.Type);
                ret = true;
            }
        }
    }

    return ret;
}

static SDL_INLINE double getWPHeight()
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

//static SDL_INLINE double getWorldPlayerX()
//{
//    return WorldPlayer[1].Location.X;
//}

//static SDL_INLINE double getWorldPlayerY()
//{
//    return WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - getWPHeight();
//}

static SDL_INLINE double getWorldPlayerCenterX()
{
    return WorldPlayer[1].Location.X + WorldPlayer[1].Location.Width / 2;
}

static SDL_INLINE double getWorldPlayerCenterY()
{
    return WorldPlayer[1].Location.Y - 10 + WorldPlayer[1].Location.Height - getWPHeight() / 2;
}

void WorldLoop()
{
    // again, SOON port the shared allocation from devel
    // Keep them static to don't re-alloc them for every iteration
    static WorldPathPtrArr parr;
    static WorldLevelPtrArr larr;
    static WorldLevelPtrArr larr2;

    // Reserve 20 elements per every array
    if(parr.capacity() < 20)
        parr.reserve(20);
    if(larr2.capacity() < 20)
        larr2.reserve(20);
    if(larr2.capacity() < 20)
        larr2.reserve(20);

    bool musicReset = false;
    Location_t tempLocation;
    int A = 0;
    int B = 0;
    if(SingleCoop > 0)
        SingleCoop = 1;
    vScreenX[1] = -(WorldPlayer[1].Location.X + WorldPlayer[1].Location.Width / 2.0) + vScreen[1].Width / 2.0;
    vScreenY[1] = -(WorldPlayer[1].Location.Y + WorldPlayer[1].Location.Height / 2.0) + vScreen[1].Height / 2.0 + 32;
    if(numPlayers > 2)
        numPlayers = 1;
    for(B = 1; B <= numPlayers; B++)
    {
        if(Player[B].Mount == 2)
            Player[B].Mount = 0;
    }

    speedRun_tick();
    UpdateGraphics2();

    if(!Controls::Update())
    {
        if(g_config.NoPauseReconnect || !g_compatibility.allow_DropAdd)
            QuickReconnectScreen::g_active = true;
        else
            PauseGame(PauseCode::Reconnect, 0);
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

            treeWorldLevelQuery(WorldPlayer[1].Location, larr, false);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
            {
                WorldLevel_t &l = *t;
                if(CheckCollision(WorldPlayer[1].Location, l.Location))
                {
                    curWorldLevel = l.index;
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

        bool pausePress = Player[1].Controls.Start || SharedControls.Pause;

        treeWorldLevelQuery(tempLocation, larr, true);
        //for(A = 1; A <= numWorldLevels; A++)
        for(auto *t : larr)
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

        if(pausePress)
        {
            if(Player[1].UnStart)
                PauseGame(PauseCode::PauseGame, 1);
        }

        if(Player[1].Controls.Up)
        {
            tempLocation.Y -= 32;
            treeWorldPathQuery(tempLocation, parr, true);
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : parr)
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 1;
                    break;
                }
            }

            treeWorldLevelQuery(tempLocation, larr, true);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
            {
                WorldLevel_t &level = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, level.Location) && level.Active)
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
            treeWorldPathQuery(tempLocation, parr, true);
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : parr)
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 2;
                    break;
                }
            }

            treeWorldLevelQuery(tempLocation, larr, true);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
            {
                WorldLevel_t &level = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, level.Location) && level.Active)
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
            treeWorldPathQuery(tempLocation, parr, true);
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : parr)
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 3;
                    break;
                }
            }

            treeWorldLevelQuery(tempLocation, larr, true);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
            {
                WorldLevel_t &level = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, level.Location) && level.Active)
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
            treeWorldPathQuery(tempLocation, parr, true);
            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : parr)
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 4;
                    break;
                }
            }

            treeWorldLevelQuery(tempLocation, larr, true);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
            {
                WorldLevel_t &level = *t;
                if(WorldPlayer[1].Move == 0)
                {
                    if(CheckCollision(tempLocation, level.Location) && level.Active)
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
            treeWorldLevelQuery(tempLocation, larr, true);
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : larr)
            {
                WorldLevel_t &level = *t;
                if(CheckCollision(tempLocation, level.Location))
                {
#if 0 // Moved into the handler of level ending
                    if(int(level.WarpX) != -1)
                        WorldPlayer[1].Location.X = level.WarpX;

                    if(int(level.WarpY) != -1)
                        WorldPlayer[1].Location.Y = level.WarpY;

                    if(int(level.WarpY) != -1 || int(level.WarpX) != -1)
                    {
                        LevelBeatCode = 6;
                        treeWorldLevelQuery(WorldPlayer[1].Location, larr2, true);
                        //for(B = 1; B <= numWorldLevels; B++)
                        for(auto *t2 : larr2)
                        {
                            WorldLevel_t &level2 = *t2;
                            if(CheckCollision(WorldPlayer[1].Location, level2.Location))
                            {
                                level2.Active = true;
                                curWorldLevel = level2.index;
                            }
                        }
                    }
#endif

                    if(!level.FileName.empty() && level.FileName != ".lvl" && level.FileName != ".lvlx")
                    {
                        addMissingLvlSuffix(level.FileName);
                        if(Files::fileExists(SelectWorld[selWorld].WorldPath + level.FileName))
                        {
                            StartWarp = level.StartWarp;
                            StopMusic();
                            PlaySound(SFX_LevelSelect);
                            g_worldScreenFader.setupFader(2, 0, 65, ScreenFader::S_RECT,
                                                          true,
                                                          getWorldPlayerCenterX(), getWorldPlayerCenterY(), 1);
                            worldWaitForFade();

                            SoundPause[26] = 200;
                            curWorldLevel = level.index;
                            LevelSelect = false;

                            ClearLevel();

                            std::string levelPath = SelectWorld[selWorld].WorldPath + level.FileName;
                            if(!OpenLevel(levelPath))
                            {
                                delayedMusicStart(); // Allow music being started
                                MessageText = fmt::format_ne("ERROR: Can't open \"{0}\": file doesn't exist or corrupted.", level.FileName);
                                PauseGame(PauseCode::Message);
                                ErrorQuit = true;
                            }

                            GameThing(1000, 3);

                            break;
                        }
                    }
                    else if(int(level.WarpX) != -1 || int(level.WarpY) != -1)
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
                        if(int(level.WarpX) != -1)
                            WorldPlayer[1].Location.X = level.WarpX;
                        if(int(level.WarpY) != -1)
                            WorldPlayer[1].Location.Y = level.WarpY;

                        LevelBeatCode = 6;
                        treeWorldLevelQuery(WorldPlayer[1].Location, larr2, true);
                        //for(B = 1; B <= numWorldLevels; B++)
                        for(auto *t2 : larr2)
                        {
                            WorldLevel_t &level2 = *t2;
                            if(CheckCollision(WorldPlayer[1].Location, level2.Location))
                            {
                                level2.Active = true;
                                curWorldLevel = level2.index;
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
        else
        {
            if(WorldPlayer[1].Frame == 5)
                WorldPlayer[1].Frame = 4;
            if(WorldPlayer[1].Frame == 3)
                WorldPlayer[1].Frame = 2;
        }
        if(WorldPlayer[1].Move3 && WorldPlayer[1].Move == 0)
        {
            WorldPlayer[1].Move3 = false;
            PlaySound(SFX_Slide);
        }

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
        if(WalkAnywhere)
        {
            WorldPlayer[1].Move2 += 2;
            WorldPlayer[1].Location.Y -= 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 2)
    {
        WorldPlayer[1].Move2 += 2;
        WorldPlayer[1].Location.X -= 2;
        if(WalkAnywhere)
        {
            WorldPlayer[1].Move2 += 2;
            WorldPlayer[1].Location.X -= 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 3)
    {
        WorldPlayer[1].Move2 += 2;
        WorldPlayer[1].Location.Y += 2;
        if(WalkAnywhere)
        {
            WorldPlayer[1].Move2 += 2;
            WorldPlayer[1].Location.Y += 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
    else if(WorldPlayer[1].Move == 4)
    {
        WorldPlayer[1].Move2 += 2;
        WorldPlayer[1].Location.X += 2;
        if(WalkAnywhere)
        {
            WorldPlayer[1].Move2 += 2;
            WorldPlayer[1].Location.X += 2;
        }
        if(WorldPlayer[1].Move2 >= 32)
        {
            WorldPlayer[1].Move2 = 0;
            WorldPlayer[1].Move = 0;
            WorldPlayer[1].Move3 = true;
        }
    }
}

void LevelPath(const WorldLevel_t &Lvl, int Direction, bool Skp)
{
    WorldPathPtrArr parr;
    parr.reserve(20);
    Location_t tempLocation;
//    int A = 0;

    // Up
    if(Direction == 1 || Direction == 5)
    {
        tempLocation = Lvl.Location;
        tempLocation.X +=  4;
        tempLocation.Y +=  4;
        tempLocation.Width -= 8;
        tempLocation.Height -=  8;
        tempLocation.Y -= 32;

        treeWorldPathQuery(tempLocation, parr, false);
        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : parr)
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                }
            }
        }
    }

    // Left
    if(Direction == 2 || Direction == 5)
    {
        tempLocation = Lvl.Location;
        tempLocation.X += 4;
        tempLocation.Y += 4;
        tempLocation.Width -= 8;
        tempLocation.Height -= 8;
        tempLocation.X -= 32;

        treeWorldPathQuery(tempLocation, parr, false);
        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : parr)
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                }
            }
        }
    }

    // Down
    if(Direction == 3 || Direction == 5)
    {
        tempLocation = Lvl.Location;
        tempLocation.X += 4;
        tempLocation.Y += 4;
        tempLocation.Width -= 8;
        tempLocation.Height -= 8;
        tempLocation.Y += 32;

        treeWorldPathQuery(tempLocation, parr, false);
        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : parr)
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                }
            }
        }
    }

    // Right
    if(Direction == 4 || Direction == 5)
    {
        tempLocation = Lvl.Location;
        tempLocation.X += 4;
        tempLocation.Y += 4;
        tempLocation.Width -= 8;
        tempLocation.Height -= 8;
        tempLocation.X += 32;

        treeWorldPathQuery(tempLocation, parr, false);
        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : parr)
        {
            WorldPath_t &path = *t;
            if(!path.Active)
            {
                if(CheckCollision(tempLocation, path.Location))
                {
                    PathPath(path, Skp);
                }
            }
        }
    }
}

void PathPath(WorldPath_t &Pth, bool Skp)
{
    WorldPathPtrArr parr;
    WorldLevelPtrArr larr;
    ScenePtrArr sarr;
    parr.reserve(20);
    larr.reserve(20);
    sarr.reserve(20);
    //int A = 0;
    int B = 0;

    Location_t tempLocation;
    tempLocation = Pth.Location;
    tempLocation.X += 4;
    tempLocation.Y += 4;
    tempLocation.Width -= 8;
    tempLocation.Height -= 8;

    treeWorldSceneQuery(tempLocation, sarr, true);
    //for(A = 1; A <= numScenes; A++)
    for(auto *t : sarr)
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
        vScreenX[1] = -(Pth.Location.X + Pth.Location.Width / 2.0) + vScreen[1].Width / 2.0;
        vScreenY[1] = -(Pth.Location.Y + Pth.Location.Height / 2.0) + vScreen[1].Height / 2.0;
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

        treeWorldPathQuery(tempLocation, parr, true);
        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : parr)
        {
            WorldPath_t &path = *t;
            D_pLogDebug("Found path activity: %d", (int)path.Active);
            if(!path.Active)
            {
                D_pLogDebugNA("Collision with path...");
                if(CheckCollision(tempLocation, path.Location))
                {
                    D_pLogDebugNA("Collision with path FOUND...");
                    PathPath(path, Skp);
                }
            }
        }

        treeWorldLevelQuery(tempLocation, larr, true);
        //for(A = 1; A <= numWorldLevels; A++)
        for(auto *t : larr)
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
                        vScreenX[1] = -(lev.Location.X + lev.Location.Width / 2.0) + vScreen[1].Width / 2.0;
                        vScreenY[1] = -(lev.Location.Y + lev.Location.Height / 2.0) + vScreen[1].Height / 2.0;
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
        PGE_Delay(1);
    } while(C < 24);

    resetFrameTimer();
}
