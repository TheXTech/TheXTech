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
#include "../player.h"
#include "../main/trees.h"
#include "../core/events.h"
#include "../compat.h"
#include "../config.h"
#include "world_globals.h"
#include "level_file.h"
#include "speedrunner.h"
#include "screen_quickreconnect.h"
#include "screen_connect.h"

#include "../pseudo_vb.h"

#include "global_dirs.h"

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

static SDL_INLINE bool s_worldUpdateMusic(const Location_t &loc)
{
    bool ret = false;

    for(auto *t : treeWorldMusicQuery(loc, false))
    {
        WorldMusic_t &mus = *t;
        if(CheckCollision(loc, mus.Location))
        {
            if(g_isWorldMusicNotSame(mus))
            {
                g_playWorldMusic(mus);
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
    bool musicReset = false;
    Location_t tempLocation;
    int A = 0;
    int B = 0;
    bool allowFastMove = (g_config.worldMapFastMove || g_config.worldMapFastMove) && g_speedRunnerMode < SPEEDRUN_MODE_2;

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
        if(g_config.NoPauseReconnect || !g_compatibility.pause_on_disconnect)
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

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(WorldPlayer[1].Location, SORTMODE_NONE))
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

        //for(A = 1; A <= numWorldLevels; A++)
        for(auto *t : treeWorldLevelQuery(tempLocation, SORTMODE_Z))
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
                PauseGame(PauseCode::PauseScreen, i);
            // only allow P1 to pause if multiplayer pause controls disabled
            if(!g_compatibility.multiplayer_pause_controls)
                break;
        }

        if(Player[1].Controls.Up)
        {
            tempLocation.Y -= 32;

            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : treeWorldPathQuery(tempLocation, SORTMODE_ID))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 1;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
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

            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : treeWorldPathQuery(tempLocation, SORTMODE_ID))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 2;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
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

            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : treeWorldPathQuery(tempLocation, SORTMODE_ID))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 3;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
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

            //for(A = 1; A <= numWorldPaths; A++)
            for(auto *t : treeWorldPathQuery(tempLocation, SORTMODE_ID))
            {
                WorldPath_t &path = *t;
                if(CheckCollision(tempLocation, path.Location) && path.Active)
                {
                    WorldPlayer[1].Move = 4;
                    break;
                }
            }

            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
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
            //for(A = 1; A <= numWorldLevels; A++)
            for(auto *t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
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
                        //for(B = 1; B <= numWorldLevels; B++)
                        for(auto *t2 : treeWorldLevelQuery(WorldPlayer[1].Location, SORTMODE_ID))
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
                        std::string levelPath = g_dirEpisode.resolveFileCaseExistsAbs(level.FileName);

                        if(!levelPath.empty())
                        {
                            // save which characters were present at level start
                            if(SwapCharAllowed())
                            {
                                pLogDebug("Save drop/add characters configuration at GameLoop()");
                                ConnectScreen::SaveChars();
                            }

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

                            delayedMusicReset(); // Reset delayed music to prevent unexpected behaviour at loaded level

                            ClearLevel();

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
                        else
                        {
                            pLogWarning("Level file name \"%s\" at %d x %d (id=%d) was not found (directory %s)",
                                        level.FileName.c_str(),
                                        (int)level.Location.X,
                                        (int)level.Location.Y,
                                        level.Type,
                                        g_dirEpisode.getCurDir().c_str()
                            );
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

                        //for(B = 1; B <= numWorldLevels; B++)
                        for(auto *t2 : treeWorldLevelQuery(WorldPlayer[1].Location, SORTMODE_ID))
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
        tempLocation = Lvl.Location;
        tempLocation.X +=  4;
        tempLocation.Y +=  4;
        tempLocation.Width -= 8;
        tempLocation.Height -=  8;
        tempLocation.Y -= 32;

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : treeWorldPathQuery(tempLocation, SORTMODE_NONE))
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

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : treeWorldPathQuery(tempLocation, SORTMODE_NONE))
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

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : treeWorldPathQuery(tempLocation, SORTMODE_NONE))
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

        //for(A = 1; A <= numWorldPaths; A++)
        for(auto *t : treeWorldPathQuery(tempLocation, SORTMODE_NONE))
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
    for(auto *t : treeWorldLevelQuery(tempLocation, false))
    {
        WorldLevel_t& level = *t;
        if(CheckCollision(tempLocation, level.Location) && level.Active)
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

        for(auto *t : treeWorldPathQuery(tempLocation, false))
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

        for(auto *t : treeWorldLevelQuery(tempLocation, false))
        {
            WorldLevel_t& level = *t;
            if(CheckCollision(tempLocation, level.Location) && level.Active)
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
    tempLocation = Pth.Location;
    tempLocation.X += 4;
    tempLocation.Y += 4;
    tempLocation.Width -= 8;
    tempLocation.Height -= 8;

    //for(A = 1; A <= numScenes; A++)
    for(auto *t : treeWorldSceneQuery(tempLocation, SORTMODE_ID))
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
        for(auto *t : treeWorldLevelQuery(tempLocation, SORTMODE_ID))
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
