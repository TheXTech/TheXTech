/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NO_SDL
#include <SDL2/SDL_timer.h>
#else
#include "SDL_supplement.h"
#endif

#include "globals.h"
#include "layers.h"
#include "effect.h"
#include "collision.h"
#include "npc.h"
#include "sound.h"
#include "graphics.h"
#include "game_main.h"
#include "compat.h"
#include "frame_timer.h"
#include "main/speedrunner.h"
#include "compat.h"
#include "editor/editor.h"
#include "blocks.h"
#include "main/trees.h"

int numLayers = 0;
RangeArr<Layer_t, 0, maxLayers> Layer;

int numEvents = 0;
RangeArr<Events_t, 0, maxEvents> Events;

RangeArr<std::string, 1, maxEvents> NewEvent;
RangeArrI<int, 1, maxEvents, 0> newEventDelay;
int newEventNum = 0;

static SDL_INLINE bool equalCase(const std::string &x, const std::string &y)
{
    return (SDL_strcasecmp(x.c_str(), y.c_str()) == 0);
}

void ShowLayer(std::string LayerName, bool NoEffect)
{
    int A = 0;
    int B = 0;
    Location_t tempLocation;

    if(LayerName.empty())
        return;

    for(int L = 0; L <= maxLayers; L++)
    {
        if(!equalCase(Layer[L].Name, LayerName))
            continue;

        Layer[L].Hidden = false;
        if(Layer[L].Name == "Destroyed Blocks")
            Layer[L].Hidden = true;
        if(Layer[L].Name == "Spawned NPCs")
            Layer[L].Hidden = false;

        for (int A : Layer[L].NPCs)
        {
            if(NPC[A].Hidden)
            {
                if(!NoEffect && !NPC[A].Generator)
                {
                    tempLocation = NPC[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }

                if(!LevelEditor)
                {
                    if(!NPCWontHurt[NPC[A].Type] && !NPCIsABonus[NPC[A].Type] && NPC[A].Active)
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(CheckCollision(Player[B].Location, NPC[A].Location))
                                Player[B].Immune = 120;
                        }
                    }
                }
            }
            NPC[A].Hidden = false;
            NPC[A].GeneratorActive = true;
            NPC[A].Reset[1] = true;
            NPC[A].Reset[2] = true;
            if(NPC[A].Generator == false)
            {
                NPC[A].Active = true;
                NPC[A].TimeLeft = 1;
            }
            CheckSectionNPC(A);
        }

        for (int A : Layer[L].blocks)
        {
            // If Not (Block(A).DefaultType = 0 And Block(A).Layer = "Destroyed Blocks") Then
            if(Block[A].Hidden == true)
            {
                if(!NoEffect && !Block[A].Invis)
                {
                    tempLocation = Block[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Block[A].Hidden = false;
        }

        for (int A : Layer[L].BGOs)
        {
            if(Background[A].Hidden)
            {
                if(!NoEffect)
                {
                    tempLocation = Background[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Background[A].Hidden = false;
        }

        for (int A : Layer[L].warps)
        {
            Warp[A].Hidden = false;
        }

        for (int A : Layer[L].waters)
        {
            Water[A].Hidden = false;
        }
    }

    if(LayerName == "Destroyed Blocks")
    {
        for(A = 1; A <= numBlock; A++)
        {
            if(Block[A].DefaultType > 0)
            {
                if(Block[A].Layer == LayerName)
                    Block[A].Layer = "Default";
                Block[A].Special = Block[A].DefaultSpecial;
                Block[A].Type = Block[A].DefaultType;
                syncLayers_Block(A);
            }
        }
    }
}

bool ExistsLayer(const std::string LayerName)
{
    if(LayerName.empty())
        return false;

    if(equalCase(LayerName, "Default"))
        return true;

    for(int A = 0; A <= maxLayers; A++)
    {
        if(equalCase(Layer[A].Name, LayerName))
            return true;
    }

    return false;
}

void HideLayer(std::string LayerName, bool NoEffect)
{
    int A = 0;
    Location_t tempLocation;
    if(LayerName.empty())
        return;

    for(int L = 0; L <= maxLayers; L++)
    {
        if(!equalCase(Layer[L].Name, LayerName))
            continue;

        Layer[L].Hidden = true;

        for (int A : Layer[L].NPCs)
        {
            if(!NPC[A].Hidden)
            {
                if(!NoEffect && !NPC[A].Generator)
                {
                    tempLocation = NPC[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            NPC[A].Hidden = true;
            if(!NPC[A].Generator)
            {
                Deactivate(A);
            }
        }

        for (int A : Layer[L].blocks)
        {
            if(!Block[A].Hidden)
            {
                if(!NoEffect && !Block[A].Invis)
                {
                    tempLocation = Block[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Block[A].Hidden = true;
        }

        for (int A : Layer[L].BGOs)
        {
            if(!Background[A].Hidden)
            {
                if(!NoEffect)
                {
                    tempLocation = Background[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Background[A].Hidden = true;
        }

        for (int A : Layer[L].warps)
            Warp[A].Hidden = true;

        for (int A : Layer[L].waters)
            Water[A].Hidden = true;
    }
}

void SetLayer(std::string /*LayerName*/)
{
    // Unused
}

bool RenameLayer(const std::string OldName, const std::string NewName)
{
    if(OldName.empty())
        return false;
    if(NewName.empty())
        return false;

    int A = 0;

    for(A = 0; A <= maxLayers; A++)
    {
        if(equalCase(Layer[A].Name, NewName))
            return false;
    }

    for(int L = 0; L <= maxLayers; L++)
    {
        if(!equalCase(Layer[L].Name, OldName))
            continue;

        Layer[L].Name = NewName;

        for (int A : Layer[L].NPCs)
            NPC[A].Layer = NewName;

        for (int A : Layer[L].blocks)
            Block[A].Layer = NewName;

        for (int A : Layer[L].BGOs)
            Background[A].Layer = NewName;

        for (int A : Layer[L].warps)
            Warp[A].Layer = NewName;

        for (int A : Layer[L].waters)
            Water[A].Layer = NewName;
    }

    for(A = 1; A <= numNPCs; A++)
    {
        if(equalCase(NPC[A].AttLayer, OldName))
            NPC[A].AttLayer = NewName;
    }

    for(A = 0; A <= numEvents; A++)
    {
        for(std::string& l : Events[A].HideLayer)
        {
            if(equalCase(l, OldName))
                l = NewName;
        }

        for(std::string& l : Events[A].ShowLayer)
        {
            if(equalCase(l, OldName))
                l = NewName;
        }

        for(std::string& l : Events[A].ToggleLayer)
        {
            if(equalCase(l, OldName))
                l = NewName;
        }

        if(equalCase(Events[A].MoveLayer, OldName))
            Events[A].MoveLayer = NewName;
    }

    if(equalCase(EditorCursor.Layer, OldName))
        EditorCursor.Layer = NewName;

    return true;
}

bool DeleteLayer(const std::string LayerName, bool killall)
{
    if(LayerName.empty())
        return false;

    int A = 0;

    for(A = numNPCs; A >= 1; A--)
    {
        if(equalCase(NPC[A].Layer, LayerName))
        {
            if (killall)
                KillNPC(A, 9);
            else
                NPC[A].Layer.clear();
        }
        if(equalCase(NPC[A].AttLayer, LayerName))
            NPC[A].AttLayer.clear();
    }

    for(A = numBlock; A >= 1; A--)
    {
        if(equalCase(Block[A].Layer, LayerName))
        {
            if (killall)
                KillBlock(A, false);
            else
                Block[A].Layer.clear();
        }
    }

    for(A = numWarps; A >= 1; A--)
    {
        if(equalCase(Warp[A].Layer, LayerName))
        {
            if (killall)
                KillWarp(A);
            else
                Warp[A].Layer.clear();
        }
    }

    for(A = numBackground; A >= 1; A--)
    {
        if(equalCase(Background[A].Layer, LayerName))
        {
            if (killall)
            {
                Background[A] = Background[numBackground];
                numBackground --;
            }
            else
                Background[A].Layer.clear();
        }
    }

    for(A = numWater; A >= 1; A--)
    {
        if(equalCase(Water[A].Layer, LayerName))
        {
            if (killall)
            {
                Water[A] = Water[numWater];
                numWater --;
            }
            else
                Water[A].Layer.clear();
        }
    }

    for(A = 0; A <= numEvents; A++)
    {
        for (auto it = Events[A].HideLayer.end(); it != Events[A].HideLayer.begin();)
        {
            it--;
            if (equalCase(*it, LayerName))
                it = Events[A].HideLayer.erase(it);
        }

        for (auto it = Events[A].ShowLayer.end(); it != Events[A].ShowLayer.begin();)
        {
            it--;
            if (equalCase(*it, LayerName))
                it = Events[A].ShowLayer.erase(it);
        }

        for (auto it = Events[A].ToggleLayer.end(); it != Events[A].ToggleLayer.begin();)
        {
            it--;
            if (equalCase(*it, LayerName))
                it = Events[A].ToggleLayer.erase(it);
        }

        if(equalCase(Events[A].MoveLayer, LayerName))
            Events[A].MoveLayer.clear();
    }

    for(A = numLayers; A >= 0; A--)
    {
        if(equalCase(Layer[A].Name, LayerName))
        {
            for (int B = A; B <= numLayers - 1; B++)
                std::swap(Layer[B], Layer[B+1]);

            Layer[numLayers] = Layer_t();
            numLayers --;
        }
    }

    if(equalCase(EditorCursor.Layer, LayerName))
        EditorCursor.Layer.clear();

    return true;
}

void InitializeEvent(Events_t& event)
{
    event = Events_t();
    for (int i = 0; i <= maxSections; i++)
    {
        event.section[i].music_id = EventSection_t::LESet_Nothing;
        event.section[i].background_id = EventSection_t::LESet_Nothing;
        event.section[i].position.X = EventSection_t::LESet_Nothing;
    }
}

bool ExistsEvent(const std::string EventName)
{
    if(EventName.empty())
        return false;

    for(int A = 0; A <= maxEvents; A++)
    {
        if(equalCase(Events[A].Name, EventName))
            return true;
    }

    return false;
}

bool RenameEvent(const std::string OldName, const std::string NewName)
{
    if(OldName.empty())
        return false;
    if(NewName.empty())
        return false;

    int A = 0;

    for(A = 0; A <= maxEvents; A++)
    {
        if(equalCase(Events[A].Name, NewName))
            return false;
    }

    for(A = 0; A <= maxEvents; A++)
    {
        if(equalCase(Events[A].Name, OldName))
            Events[A].Name = NewName;
    }

    for(A = 1; A <= numNPCs; A++)
    {
        if(equalCase(NPC[A].TriggerTalk, OldName))
            NPC[A].TriggerTalk = NewName;
        if(equalCase(NPC[A].TriggerDeath, OldName))
            NPC[A].TriggerDeath = NewName;
        if(equalCase(NPC[A].TriggerLast, OldName))
            NPC[A].TriggerLast = NewName;
        if(equalCase(NPC[A].TriggerActivate, OldName))
            NPC[A].TriggerActivate = NewName;
    }

    for(A = 1; A <= numBlock; A++)
    {
        if(equalCase(Block[A].TriggerHit, OldName))
            Block[A].TriggerHit = NewName;
        if(equalCase(Block[A].TriggerDeath, OldName))
            Block[A].TriggerDeath = NewName;
        if(equalCase(Block[A].TriggerLast, OldName))
            Block[A].TriggerLast = NewName;
    }


    for(A = 0; A <= numEvents; A++)
    {
        if(equalCase(Events[A].TriggerEvent, OldName))
            Events[A].TriggerEvent = NewName;
    }

    return true;
}

bool DeleteEvent(const std::string EventName)
{
    if(EventName.empty())
        return false;

    int A = 0;

    for(A = 1; A <= numNPCs; A++)
    {
        if(equalCase(NPC[A].TriggerTalk, EventName))
            NPC[A].TriggerTalk.clear();
        if(equalCase(NPC[A].TriggerDeath, EventName))
            NPC[A].TriggerDeath.clear();
        if(equalCase(NPC[A].TriggerLast, EventName))
            NPC[A].TriggerLast.clear();
        if(equalCase(NPC[A].TriggerActivate, EventName))
            NPC[A].TriggerActivate.clear();
    }

    for(A = 1; A <= numBlock; A++)
    {
        if(equalCase(Block[A].TriggerHit, EventName))
            Block[A].TriggerHit.clear();
        if(equalCase(Block[A].TriggerDeath, EventName))
            Block[A].TriggerDeath.clear();
        if(equalCase(Block[A].TriggerLast, EventName))
            Block[A].TriggerLast.clear();
    }


    for(A = 0; A <= numEvents; A++)
    {
        if(equalCase(Events[A].TriggerEvent, EventName))
            Events[A].TriggerEvent.clear();
    }

    for(A = numEvents; A >= 0; A--)
    {
        if(equalCase(Events[A].Name, EventName))
        {
            for (int B = A; B <= numEvents - 1; B++)
                std::swap(Events[B], Events[B+1]);

            Events[numEvents] = Events_t();
            numEvents --;
        }
    }

    return true;
}

void ProcEvent(std::string EventName, bool NoEffect)
{
    // this is for events that have just been triggered
    int A = 0;
    int B = 0;
    int C = 0;
    int D = 0;
    int plr = 0;
    bool tempBool = false;
    Location_t tempLevel;
    vScreen_t screenLoc;
    double tX = 0;
    double tY = 0;
    // Ignore vanilla autoscroll if newer way has been used
    bool autoScrollerChanged = false;

    if(EventName.empty() || LevelEditor)
        return;

    for(A = 0; A <= numEvents; A++)
    {
        auto &evt = Events[A];
        if(equalCase(EventName, evt.Name))
        {
            if(equalCase(EventName, "Boss Dead"))
                speedRun_bossDeadEvent();

            for(B = 0; B <= numSections; B++)
            {
                /* Music change */
                auto &s = evt.section[B];

                bool musicChanged = false;
                if(s.music_id == EventSection_t::LESet_ResetDefault)
                {
                    bgMusic[B] = bgMusicREAL[B];
                    musicChanged = true;
                }
                else if(s.music_id != EventSection_t::LESet_Nothing)
                {
                    bgMusic[B] = s.music_id;
                    musicChanged = true;
                }

                if(musicChanged && (B == Player[1].Section || (numPlayers == 2 && B == Player[2].Section)))
                    StartMusic(B);

                /* Background change */
                if(s.background_id == EventSection_t::LESet_ResetDefault)
                    Background2[B] = Background2REAL[B];
                else if(s.background_id != EventSection_t::LESet_Nothing)
                    Background2[B] = s.background_id;

                /* Per-Section autoscroll setup */
                if(s.autoscroll)
                {
                    autoScrollerChanged = true;
                    AutoX[B] = s.autoscroll_x;
                    AutoY[B] = s.autoscroll_y;
                }

                /* Resize the section noundaries */
                if(int(s.position.X) == EventSection_t::LESet_ResetDefault)
                    level[B] = LevelREAL[B];
                else if(int(s.position.X) != EventSection_t::LESet_Nothing)
                {
                    tempLevel = level[B];
                    level[B] = s.position;

                    if(!evt.AutoStart && !equalCase(evt.Name, "Level - Start"))
                    {
                        for(C = 1; C <= numPlayers; C++)
                        {
                            // If .Section = B Then
                            Player[C].Section = B;
                            tempBool = false;
                            if(Player[C].Location.X + Player[C].Location.Width >= level[B].X)
                            {
                                if(Player[C].Location.X <= level[B].Width)
                                {
                                    if(Player[C].Location.Y + Player[C].Location.Height >= level[B].Y)
                                    {
                                        if(Player[C].Location.Y <= level[B].Height)
                                        {
                                            tempBool = true; // Check to see if player is still in section after resizing
                                            plr = C;
                                        }
                                    }
                                }
                            }
                            if(!tempBool)
                            {
                                for(D = 1; D <= numPlayers; D++)
                                {
                                    if(D != C && Player[D].Section == B)
                                    {
                                        if(Player[D].Location.X + Player[D].Location.Width >= level[B].X)
                                        {
                                            if(Player[D].Location.X <= level[B].Width)
                                            {
                                                if(Player[D].Location.Y + Player[D].Location.Height >= level[B].Y)
                                                {
                                                    if(Player[D].Location.Y <= level[B].Height) // Move to another player who is still in the section
                                                    {
                                                        Player[C].Location.X = Player[D].Location.X + Player[D].Location.Width / 2.0 - Player[C].Location.Width / 2.0;
                                                        Player[C].Location.Y = Player[D].Location.Y + Player[D].Location.Height - Player[C].Location.Height;
                                                        Player[C].Effect = 9;
                                                        Player[C].Effect2 = D;
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            // End If
                        }
                    }

                    if(!equalCase(evt.Name, "Level - Start"))
                    {
                        C = plr;
                        if(numPlayers == 2 && DScreenType != 5)
                        {
                            tX = vScreenX[C];
                            tY = vScreenY[C];
                            level[B] = tempLevel;
                            screenLoc = vScreen[C];
                            SoundPause[13] = 10;
                            DynamicScreen();
                            GetvScreenAverage();
                            qScreen = true;
                            qScreenX[1] = vScreenX[1];
                            qScreenY[1] = vScreenY[1];
                            if(int(screenLoc.Width) == 400)
                            {
                                if(qScreenX[1] < tX + screenLoc.Left)
                                    qScreenX[1] = qScreenX[1] + 200;
                                else
                                    qScreenX[1] = qScreenX[1] - 200;
                            }
                            if(int(screenLoc.Height) == 300)
                            {
                                if(qScreenY[1] < tY + screenLoc.Top)
                                    qScreenY[1] = qScreenY[1] + 150;
                                else
                                    qScreenY[1] = qScreenY[1] - 150;
                            }
                            if(-qScreenX[1] < level[Player[C].Section].X)
                                qScreenX[1] = -level[Player[C].Section].X;
                            if(-qScreenX[1] + frmMain.ScaleWidth > level[Player[C].Section].Width)
                                qScreenX[1] = -(level[Player[C].Section].Width - ScreenW);
                            if(-qScreenY[1] < level[Player[C].Section].Y)
                                qScreenY[1] = -level[Player[C].Section].Y;
                            if(-qScreenY[1] + frmMain.ScaleHeight > level[Player[C].Section].Height)
                                qScreenY[1] = -(level[Player[C].Section].Height - ScreenH);
                            level[B] = s.position;
                        }
                        else
                        {
                            qScreen = true;
                            qScreenX[1] = vScreenX[1];
                            qScreenY[1] = vScreenY[1];
                            qScreenLoc[1] = vScreen[1];
                        }

                        resetFrameTimer();
                    }
                }
            }

            for(auto &l : evt.HideLayer)
            {
                HideLayer(l, NoEffect ? true : evt.LayerSmoke);
            }

            for(auto &l : evt.ShowLayer)
            {
                ShowLayer(l, NoEffect ? true : evt.LayerSmoke);
            }

            for(auto &l : evt.ToggleLayer)
            {
                if(l.empty())
                    continue;

                for(C = 0; C <= numLayers; C++)
                {
                    if(Layer[C].Name == l)
                    {
                        if(Layer[C].Hidden)
                            ShowLayer(Layer[C].Name, evt.LayerSmoke);
                        else
                            HideLayer(Layer[C].Name, evt.LayerSmoke);
                    }
                }
            }

#if 0 // Obsolete, replaced with a code above
            for(B = 0; B <= 20; B++)
            {
                if(NoEffect == true)
                {
                    HideLayer(evt.HideLayer[B], NoEffect);
                    ShowLayer(evt.ShowLayer[B], NoEffect);
                }
                else
                {
                    HideLayer(evt.HideLayer[B], evt.LayerSmoke);
                    ShowLayer(evt.ShowLayer[B], evt.LayerSmoke);
                }

                if(!(evt.ToggleLayer[B] == ""))
                {
                    for(C = 0; C <= maxLayers; C++)
                    {
                        if(Layer[C].Name == evt.ToggleLayer[B])
                        {
                            if(Layer[C].Hidden)
                            {
                                ShowLayer(Layer[C].Name, evt.LayerSmoke);
                            }
                            else
                            {
                                HideLayer(Layer[C].Name, evt.LayerSmoke);
                            }
                        }
                    }
                }
            }
#endif

            if(!evt.MoveLayer.empty())
            {
                for(B = 0; B <= maxLayers; B++)
                {
                    if(Layer[B].Name == evt.MoveLayer)
                    {
                        Layer[B].EffectStop = true;
                        Layer[B].SpeedX = evt.SpeedX;
                        Layer[B].SpeedY = evt.SpeedY;
                        if(Layer[B].SpeedX == 0.f && Layer[B].SpeedY == 0.f)
                        {
                            // stop layer
                            Layer[B].EffectStop = false;
                            for(int C : Layer[B].blocks)
                            {
                                Block[C].Location.SpeedX = double(Layer[B].SpeedX);
                                Block[C].Location.SpeedY = double(Layer[B].SpeedY);
                            }
                            if(g_compatibility.enable_climb_bgo_layer_move)
                            {
                                for(int C : Layer[B].BGOs)
                                {
                                    if(BackgroundFence[Background[C].Type])
                                    {
                                        Background[C].Location.SpeedX = double(Layer[B].SpeedX);
                                        Background[C].Location.SpeedY = double(Layer[B].SpeedY);
                                    }
                                }
                            }
                            for(int C : Layer[B].NPCs)
                            {
                                if(NPCIsAVine[NPC[C].Type] || NPC[C].Type == 91)
                                {
                                    NPC[C].Location.SpeedX = 0;
                                    NPC[C].Location.SpeedY = 0;
                                }
                            }
                        }
                    }
                }
            }

            if(g_compatibility.fix_autoscroll_speed)
            {
                if(!autoScrollerChanged)
                {
                    // Do set the autoscrool when non-zero values only, don't zero by other autoruns
                    if(evt.AutoX != 0.0 || evt.AutoY != 0.0)
                    {
                        AutoX[evt.AutoSection] = evt.AutoX;
                        AutoY[evt.AutoSection] = evt.AutoY;
                    }
                }
            }
            else // Buggy behavior, see https://github.com/Wohlstand/TheXTech/issues/44
            {
                AutoX[evt.AutoSection] = Events[evt.AutoSection].AutoX;
                AutoY[evt.AutoSection] = Events[evt.AutoSection].AutoY;
            }

            if(!evt.Text.empty())
            {
                MessageText = evt.Text;
                PauseGame(1);
                MessageText = "";
            }

            if(evt.Sound > 0)
            {
                SoundPause[evt.Sound] = 0;
                PlaySound(evt.Sound);
            }

            if(evt.EndGame == 1)
            {
                for(B = 0; B <= numSections; B++)
                    bgMusic[B] = 0;
                StopMusic();
                speedRun_bossDeadEvent();
                LevelMacroCounter = 0;
                LevelMacro = LEVELMACRO_GAME_COMPLETE_EXIT;
            }

            ForcedControls = (evt.Controls.AltJump ||
                              evt.Controls.AltRun ||
                              evt.Controls.Down ||
                              evt.Controls.Drop ||
                              evt.Controls.Jump ||
                              evt.Controls.Left ||
                              evt.Controls.Right ||
                              evt.Controls.Run ||
                              evt.Controls.Start ||
                              evt.Controls.Up);

            ForcedControl = evt.Controls;

            tempBool = false;
            if(!evt.TriggerEvent.empty())
            {
                if(std::round(evt.TriggerDelay) == 0.0)
                {
                    for(B = 0; B <= maxEvents; B++)
                    {
                        if(Events[B].Name == evt.TriggerEvent)
                        {
                            if(Events[B].TriggerEvent == evt.Name)
                                tempBool = true;
                            break;
                        }
                    }

                    if(!tempBool)
                        ProcEvent(evt.TriggerEvent);
                }
                else
                {
                    newEventNum++;
                    NewEvent[newEventNum] = evt.TriggerEvent;
                    newEventDelay[newEventNum] = vb6Round(evt.TriggerDelay * 6.5);
                }
            }
        }
    }
}

void UpdateEvents()
{
    // this is for evetns that have a delay to call other events
    // this sub also updates the screen position for autoscroll levels
    int A = 0;
    int B = 0;
    if(FreezeNPCs)
        return;

    if(!GameMenu)
    {
        for(B = 1; B <= numPlayers; B++)
        {
            if(!(Player[B].Effect == 0 || Player[B].Effect == 3 || Player[B].Effect == 9 || Player[B].Effect == 10))
                return;
        }
    }

    if(newEventNum > 0)
    {
        for(A = 1; A <= newEventNum; A++)
        {
            if(newEventDelay[A] > 0)
                newEventDelay[A]--;
            else
            {
                ProcEvent(NewEvent[A]);
                newEventDelay[A] = newEventDelay[newEventNum];
                NewEvent[A] = NewEvent[newEventNum];
                newEventNum--;
            }
        }
    }

    for(A = 0; A <= numSections; A++)
    {
        if(AutoX[A] != 0.0f || AutoY[A] != 0.0f)
        {
            level[A].X += double(AutoX[A]);
            level[A].Width += double(AutoX[A]);
            level[A].Y += double(AutoY[A]);
            level[A].Height += double(AutoY[A]);
            if(level[A].Width > LevelREAL[A].Width)
            {
                level[A].Width = LevelREAL[A].Width;
                level[A].X = LevelREAL[A].Width - 800;
            }
            if(level[A].X < LevelREAL[A].X)
            {
                level[A].Width = LevelREAL[A].X + 800;
                level[A].X = LevelREAL[A].X;
            }
            if(level[A].Height > LevelREAL[A].Height)
            {
                level[A].Height = LevelREAL[A].Height;
                level[A].Y = LevelREAL[A].Height - 800;
            }
            if(level[A].Y < LevelREAL[A].Y)
            {
                level[A].Height = LevelREAL[A].Y + 800;
                level[A].Y = LevelREAL[A].Y;
            }
        }
    }
}

void UpdateLayers()
{
    // this is mainly for moving layers
    int A = 0;
    int B = 0;
    int C = 0;

    bool FreezeLayers = false;

    if(!GameMenu)
    {
        for(B = 1; B <= numPlayers; B++)
        {
            if(!(Player[B].Effect == 0 || Player[B].Effect == 3 || Player[B].Effect == 9 || Player[B].Effect == 10))
                FreezeLayers = true;
        }
    }

    for(A = 0; A <= maxLayers; A++)
    {
        // only consider the non-empty, non-stationary layers
        if(Layer[A].Name.empty() || (Layer[A].SpeedX == 0.f && Layer[A].SpeedY == 0.f)) continue;
        if(FreezeNPCs || (FreezeLayers && Layer[A].EffectStop))
        {
            for(int B : Layer[A].blocks)
            {
                Block[B].Location.SpeedX = 0;
                Block[B].Location.SpeedY = 0;
            }

            if(g_compatibility.enable_climb_bgo_layer_move)
            {
                for(int B : Layer[A].BGOs)
                {
                    if(BackgroundFence[Background[B].Type])
                    {
                        Background[B].Location.SpeedX = 0;
                        Background[B].Location.SpeedY = 0;
                    }
                }
            }
        }
        else
        {
            // move the sort invalidation out of the loop over blocks
            // if(!Layer[A].blocks.empty() && Layer[A].SpeedX != 0.f)
            // {
            //     if(BlocksSorted == true)
            //     {
            //         for(C = (int)(-FLBlocks); C <= FLBlocks; C++)
            //         {
            //             FirstBlock[C] = 1;
            //             LastBlock[C] = numBlock;
            //         }
            //         BlocksSorted = false;
            //     }
            // }

            Layer[A].OffsetX += double(Layer[A].SpeedX);
            Layer[A].OffsetY += double(Layer[A].SpeedY);

            for(int B : Layer[A].blocks)
            {
                        Block[B].Location.X += double(Layer[A].SpeedX);
                        Block[B].Location.Y += double(Layer[A].SpeedY);
                        Block[B].Location.SpeedX = double(Layer[A].SpeedX);
                        Block[B].Location.SpeedY = double(Layer[A].SpeedY);
            }


            for(int B : Layer[A].BGOs)
            {
                        Background[B].Location.X += double(Layer[A].SpeedX);
                        Background[B].Location.Y += double(Layer[A].SpeedY);
                        if(g_compatibility.enable_climb_bgo_layer_move && BackgroundFence[Background[B].Type])
                        {
                            Background[B].Location.SpeedX = double(Layer[A].SpeedX);
                            Background[B].Location.SpeedY = double(Layer[A].SpeedY);
                        }
            }

            for(int B : Layer[A].waters)
            {
                        Water[B].Location.X += double(Layer[A].SpeedX);
                        Water[B].Location.Y += double(Layer[A].SpeedY);
            }

            for(int B : Layer[A].NPCs)
            {
                        NPC[B].DefaultLocation.X += double(Layer[A].SpeedX);
                        NPC[B].DefaultLocation.Y += double(Layer[A].SpeedY);

                        if(!NPC[B].Active || NPC[B].Generator || NPC[B].Effect != 0 ||
                           NPCIsACoin[NPC[B].Type] || NPC[B].Type == 8 || NPC[B].Type == 37 ||
                           NPC[B].Type == 51 || NPC[B].Type == 52 || NPC[B].Type == 46 ||
                           NPC[B].Type == 93 || NPC[B].Type == 74 || NPCIsAVine[NPC[B].Type] ||
                           NPC[B].Type == 192 || NPC[B].Type == 197 || NPC[B].Type == 91 ||
                           NPC[B].Type == 211 || NPC[B].Type == 256 || NPC[B].Type == 257 ||
                           NPC[B].Type == 245)
                        {
                            if(NPC[B].Type == 91 || NPC[B].Type == 211)
                            {
                                NPC[B].Location.SpeedX = double(Layer[A].SpeedX);
                                NPC[B].Location.SpeedY = double(Layer[A].SpeedY);
                            }
                            else if(NPCIsAVine[NPC[B].Type])
                            {
                                NPC[B].Location.SpeedX = double(Layer[A].SpeedX);
                                NPC[B].Location.SpeedY = double(Layer[A].SpeedY);
                            }

                            if(!NPC[B].Active)
                            {
                                NPC[B].Location.X = NPC[B].DefaultLocation.X;
                                NPC[B].Location.Y = NPC[B].DefaultLocation.Y;
                                if(NPC[B].Type == 8 || NPC[B].Type == 74 || NPC[B].Type == 93 ||
                                   NPC[B].Type == 256 || NPC[B].Type == 245)
                                    NPC[B].Location.Y += NPC[B].DefaultLocation.Height;
                                else if(NPC[B].Type == 52 && fiEqual(NPC[B].Direction, -1))
                                    NPC[B].Location.X += NPC[B].DefaultLocation.Width;
                            }
                            else
                            {
                                NPC[B].Location.X += double(Layer[A].SpeedX);
                                NPC[B].Location.Y += double(Layer[A].SpeedY);
                            }

                            if(NPC[B].Effect == 4)
                            {
                                if(NPC[B].Effect3 == 1 || NPC[B].Effect3 == 3)
                                    NPC[B].Effect2 += double(Layer[A].SpeedY);
                                else
                                    NPC[B].Effect2 += double(Layer[A].SpeedX);
                            }

                            if(!NPC[B].Active)
                            {
                                if(!NPC[B].AttLayer.empty())
                                {
                                     for(C = 1; C <= maxLayers; C++)
                                     {
                                         if(NPC[B].AttLayer == Layer[C].Name)
                                         {
                                             Layer[C].SpeedX = Layer[A].SpeedX;
                                             Layer[C].SpeedY = Layer[A].SpeedY;
                                         }
                                     }
                                 }
                             }
                        }
            }

            for(int B : Layer[A].warps)
            {
                        Warp[B].Entrance.X += double(Layer[A].SpeedX);
                        Warp[B].Entrance.Y += double(Layer[A].SpeedY);
                        Warp[B].Exit.X += double(Layer[A].SpeedX);
                        Warp[B].Exit.Y += double(Layer[A].SpeedY);
            }
        }
    }
}

void syncLayers_AllBlocks()
{
    treeLevelCleanBlockLayers();
    for (int block = 1; block <= numBlock; block++)
    {
        syncLayers_Block(block);
    }
}

void syncLayers_Block(int block)
{
    int foundLayer = false;
    for (int layer = 0; layer <= numLayers; layer++)
    {
        if (block <= numBlock && Block[block].Layer == Layer[layer].Name)
        {
            if(!foundLayer)
            {
                Block[block].LayerIndex = layer;
                Block[block].LocationInLayer = Block[block].Location;
                Block[block].LocationInLayer.X = Block[block].Location.X - Layer[layer].OffsetX;
                Block[block].LocationInLayer.Y = Block[block].Location.Y - Layer[layer].OffsetY;
                treeBlockAddLayer(layer, &Block[block]);
                foundLayer = true;
            }
            else
                treeBlockRemoveLayer(layer, &Block[block]);
            Layer[layer].blocks.insert(block);
        }
        else
        {
            Layer[layer].blocks.erase(block);
            treeBlockRemoveLayer(layer, &Block[block]);
        }
    }
    if(!foundLayer && block <= numBlock)
    {
        Block[block].LayerIndex = -1;
        Block[block].LocationInLayer = Block[block].Location;
        treeBlockAddLayer(-1, &Block[block]);
    }
    else
    {
        treeBlockRemoveLayer(-1, &Block[block]);
    }
}

void syncLayers_Block_SetHidden(int block) // set block hidden based on layer
{
    syncLayers_Block(block);
    if(Block[block].LayerIndex != -1)
        Block[block].Hidden = Layer[Block[block].LayerIndex].Hidden;
}

void syncLayers_AllNPCs()
{
    for (int npc = 1; npc <= numNPCs; npc++)
    {
        syncLayers_NPC(npc);
    }
}

void syncLayers_NPC(int npc)
{
    for (int layer = 0; layer <= numLayers; layer++)
    {
        if (npc <= numNPCs && NPC[npc].Layer == Layer[layer].Name)
            Layer[layer].NPCs.insert(npc);
        else
            Layer[layer].NPCs.erase(npc);
    }
}

void syncLayers_AllBGOs()
{
    for (int bgo = 1; bgo <= numBackground + numLocked; bgo++)
    {
        syncLayers_BGO(bgo);
    }
}

void syncLayers_BGO(int bgo)
{
    for (int layer = 0; layer <= numLayers; layer++)
    {
        if (bgo <= numBackground + numLocked && Background[bgo].Layer == Layer[layer].Name)
            Layer[layer].BGOs.insert(bgo);
        else
            Layer[layer].BGOs.erase(bgo);
    }
}

void syncLayers_Warp(int warp)
{
    for (int layer = 0; layer <= numLayers; layer++)
    {
        if (warp <= numWarps && Warp[warp].Layer == Layer[layer].Name)
            Layer[layer].warps.insert(warp);
        else
            Layer[layer].warps.erase(warp);
    }
}

void syncLayers_Water(int water)
{
    for (int layer = 0; layer <= numLayers; layer++)
    {
        if (water <= numWater && Water[water].Layer == Layer[layer].Name)
            Layer[layer].waters.insert(water);
        else
            Layer[layer].waters.erase(water);
    }
}
