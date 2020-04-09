/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "globals.h"
#include "layers_wld.h"
#include "effect.h"
#include "collision.h"
#include "npc.h"
#include "sound.h"
#include "graphics.h"
#include "game_main.h"

int numLayersWld = 0;
RangeArr<LayerWld_t, 0, maxLayersWld> LayerWld;

int numEventsWld = 0;
RangeArr<EventsWld_t, 0, maxEventsWld> EventsWld;

RangeArr<std::string, 1, maxEventsWld> NewEventWld;
RangeArrI<int, 1, maxEventsWld, 0> newEventWldDelay;
int newEventWldNum = 0;

static SDL_INLINE bool equalCase(const std::string &x, const std::string &y)
{
    return (SDL_strcasecmp(x.c_str(), y.c_str()) == 0);
}

void ShowLayerWLD(std::string LayerName, bool NoEffect)
{
    Location_t tempLocation;

    if(LayerName.empty())
        return;

    for(int A = 0; A <= maxLayersWld; A++)
    {
        if(equalCase(LayerWld[A].Name, LayerName))
            LayerWld[A].Hidden = true;
    }

    for(int A = 1; A <= numTiles; A++)
    {
        if(equalCase(Tile[A].Layer, LayerName))
        {
            if(Tile[A].Hidden == true)
            {
                if(NoEffect == false && NPC[A].Generator == false)
                {
                    tempLocation = Tile[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Tile[A].Hidden = false;
        }
    }

    for(int A = 1; A <= numScenes; A++)
    {
        if(equalCase(Scene[A].Layer, LayerName))
        {
            if(Scene[A].Hidden == true)
            {
                if(NoEffect == false)
                {
                    tempLocation = Scene[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Scene[A].Hidden = false;
        }
    }

    for(int A = 1; A <= numWorldPaths; A++)
    {
        if(equalCase(WorldPath[A].Layer, LayerName))
        {
            if(WorldPath[A].Hidden == true)
            {
                if(NoEffect == false)
                {
                    tempLocation = WorldPath[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            WorldPath[A].Hidden = false;
        }
    }

    for(int A = 1; A <= numWorldLevels; A++)
    {
        if(equalCase(WorldLevel[A].Layer, LayerName))
        {
            if(WorldLevel[A].Hidden == true)
            {
                if(NoEffect == false)
                {
                    tempLocation = WorldLevel[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            WorldLevel[A].Hidden = false;
        }
    }

    for(int A = 1; A <= numWorldMusic; A++)
    {
        if(equalCase(WorldMusic[A].Layer, LayerName))
        {
            if(WorldMusic[A].Hidden == true)
            {
                if(NoEffect == false)
                {
                    tempLocation = WorldMusic[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            WorldMusic[A].Hidden = false;
        }
    }
}

void HideLayerWLD(std::string LayerName, bool NoEffect)
{
    Location_t tempLocation;
    if(LayerName.empty())
        return;

    for(int A = 0; A <= maxLayersWld; A++)
    {
        if(equalCase(LayerWld[A].Name, LayerName))
            LayerWld[A].Hidden = true;
    }

    for(int A = 1; A <= numTiles; A++)
    {
        if(equalCase(Tile[A].Layer, LayerName))
        {
            if(Tile[A].Hidden == false)
            {
                if(NoEffect == false && NPC[A].Generator == false)
                {
                    tempLocation = Tile[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Tile[A].Hidden = true;
        }
    }

    for(int A = 1; A <= numScenes; A++)
    {
        if(equalCase(Scene[A].Layer, LayerName))
        {
            if(Scene[A].Hidden == false)
            {
                if(NoEffect == false)
                {
                    tempLocation = Scene[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Scene[A].Hidden = true;
        }
    }

    for(int A = 1; A <= numWorldPaths; A++)
    {
        if(equalCase(WorldPath[A].Layer, LayerName))
        {
            if(WorldPath[A].Hidden == false)
            {
                if(NoEffect == false)
                {
                    tempLocation = WorldPath[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            WorldPath[A].Hidden = true;
        }
    }

    for(int A = 1; A <= numWorldLevels; A++)
    {
        if(equalCase(WorldLevel[A].Layer, LayerName))
        {
            if(WorldLevel[A].Hidden == false)
            {
                if(NoEffect == false)
                {
                    tempLocation = WorldLevel[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            WorldLevel[A].Hidden =  true;
        }
    }

    for(int A = 1; A <= numWorldMusic; A++)
    {
        if(equalCase(WorldMusic[A].Layer, LayerName))
        {
            if(WorldMusic[A].Hidden == false)
            {
                if(NoEffect == false)
                {
                    tempLocation = WorldMusic[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            WorldMusic[A].Hidden = true;
        }
    }
}

void ProcEventWLD(std::string EventName, bool NoEffect)
{
    // this is for events that have just been triggered
    bool tempBool = false;

    if(EventName.empty() || LevelEditor == true)
        return;

    for(int A = 0; A <= maxEventsWld; A++)
    {
        if(equalCase(EventName, EventsWld[A].Name))
        {
            for(int B = 0; B <= 20; B++)
            {
                if(NoEffect == true)
                {
                    HideLayerWLD(EventsWld[A].HideLayer[B], NoEffect);
                    ShowLayerWLD(EventsWld[A].ShowLayer[B], NoEffect);
                }
                else
                {
                    HideLayerWLD(EventsWld[A].HideLayer[B], EventsWld[A].LayerSmoke);
                    ShowLayerWLD(EventsWld[A].ShowLayer[B], EventsWld[A].LayerSmoke);
                }

                if(!(EventsWld[A].ToggleLayer[B].empty()))
                {
                    for(int C = 0; C <= maxLayersWld; C++)
                    {
                        if(LayerWld[C].Name == EventsWld[A].ToggleLayer[B])
                        {
                            if(LayerWld[C].Hidden == true)
                            {
                                ShowLayerWLD(LayerWld[C].Name, EventsWld[A].LayerSmoke);
                            }
                            else
                            {
                                HideLayerWLD(LayerWld[C].Name, EventsWld[A].LayerSmoke);
                            }
                        }
                    }
                }
            }

            if(!EventsWld[A].MoveLayer.empty())
            {
                for(int B = 0; B <= maxLayersWld; B++)
                {
                    if(LayerWld[B].Name == EventsWld[A].MoveLayer)
                    {
                        LayerWld[B].EffectStop = true;
                        LayerWld[B].SpeedX = EventsWld[A].SpeedX;
                        LayerWld[B].SpeedY = EventsWld[A].SpeedY;
                        if(LayerWld[B].SpeedX == 0.f && LayerWld[B].SpeedY == 0.f)
                        {
                            // stop layer
                            LayerWld[B].EffectStop = false;
                            for(int I = 1; I <= numTiles; I++)
                            {
                                if(Tile[I].Layer == LayerWld[B].Name)
                                {
                                    Tile[I].Location.X = double(LayerWld[B].SpeedX);
                                    Tile[I].Location.Y = double(LayerWld[B].SpeedY);
                                }
                            }

                            for(int I = 1; I <= numScenes; I++)
                            {
                                if(Scene[I].Layer == LayerWld[B].Name)
                                {
                                     Scene[I].Location.X = double(LayerWld[B].SpeedX);
                                     Scene[I].Location.Y = double(LayerWld[B].SpeedY);
                                }
                            }

                            for(int I = 1; I <= numWorldPaths; I++)
                            {
                                if(WorldPath[I].Layer == LayerWld[B].Name)
                                {
                                     WorldPath[I].Location.X = double(LayerWld[B].SpeedX);
                                     WorldPath[I].Location.Y = double(LayerWld[B].SpeedY);
                                }
                            }

                            for(int I = 1; I <= numWorldLevels; I++)
                            {
                                if(WorldLevel[I].Layer == LayerWld[B].Name)
                                {
                                     WorldLevel[I].Location.X = double(LayerWld[B].SpeedX);
                                     WorldLevel[I].Location.Y = double(LayerWld[B].SpeedY);
                                }
                            }

                            for(int I = 1; I <= numWorldMusic; I++)
                            {
                                if(WorldMusic[I].Layer == LayerWld[B].Name)
                                {
                                      WorldMusic[I].Location.X = double(LayerWld[B].SpeedX);
                                      WorldMusic[I].Location.Y = double(LayerWld[B].SpeedY);
                                }
                            }
                        }
                    }
                }
            }

#if 0 // оно не работает пока что
            if(!EventsWld[A].Text.empty())
            {
                MessageText = EventsWld[A].Text;
                PauseGame(1);
                MessageText = "";
            }
#endif

            if(EventsWld[A].Sound > 0)
            {
                SoundPause[EventsWld[A].Sound] = 0;
                PlaySound(EventsWld[A].Sound);
            }

            ForcedControls = (EventsWld[A].Controls.Down ||
                              EventsWld[A].Controls.Left ||
                              EventsWld[A].Controls.Right ||
                              EventsWld[A].Controls.Start ||
                              EventsWld[A].Controls.Up);

            ForcedControl = EventsWld[A].Controls;

            tempBool = false;
            if(!EventsWld[A].TriggerEvent.empty())
            {
                if(int(EventsWld[A].TriggerDelay) == 0)
                {
                    for(int B = 0; B <= maxEventsWld; B++)
                    {
                        if(EventsWld[B].Name == EventsWld[A].TriggerEvent)
                        {
                            if(EventsWld[B].TriggerEvent == EventsWld[A].Name)
                                tempBool = true;
                            break;
                        }
                    }
                    if(tempBool == false)
                    {
                        ProcEventWLD(EventsWld[A].TriggerEvent);
                    }
                }
                else
                {
                    newEventWldNum++;
                    NewEventWld[newEventWldNum] = EventsWld[A].TriggerEvent;
                    newEventWldDelay[newEventWldNum] = EventsWld[A].TriggerDelay * 6.5;
                }
            }
        }
    }
}

void UpdateEventsWLD()
{
    // this is for evetns that have a delay to call other events
    // this sub also updates the screen position for autoscroll levels

    if(newEventWldNum > 0)
    {
        for(int A = 1; A <= newEventWldNum; A++)
        {
            if(newEventWldDelay[A] > 0)
                newEventWldDelay[A]--;
            else
            {
                ProcEventWLD(NewEventWld[A]);
                newEventWldDelay[A] = newEventWldDelay[newEventWldNum];
                NewEventWld[A] = NewEventWld[newEventWldNum];
                newEventWldNum--;
            }
        }
    }
}

void UpdateLayersWLD()
{
    for(int A = 0; A <= maxLayersWld; A++)
    {
         for(int B = 1; A <= numTiles; A++)
         {
             if(Tile[B].Layer == LayerWld[A].Name)
             {
                 Tile[B].Location.X += double(LayerWld[A].SpeedX);
                 Tile[B].Location.Y += double(LayerWld[A].SpeedY);
             }
         }

         for(int B = 1; A <= numScenes; A++)
         {
             if(Scene[B].Layer == LayerWld[A].Name)
             {
                  Scene[B].Location.X += double(LayerWld[A].SpeedX);
                  Scene[B].Location.Y += double(LayerWld[A].SpeedY);
             }
         }

         for(int B = 1; A <= numWorldPaths; A++)
         {
             if(WorldPath[B].Layer == LayerWld[A].Name)
             {
                  WorldPath[B].Location.X += double(LayerWld[A].SpeedX);
                  WorldPath[B].Location.Y += double(LayerWld[A].SpeedY);
             }
         }

         for(int B = 1; A <= numWorldLevels; A++)
         {
             if(WorldLevel[B].Layer == LayerWld[A].Name)
             {
                  WorldLevel[B].Location.X += double(LayerWld[A].SpeedX);
                  WorldLevel[B].Location.Y += double(LayerWld[A].SpeedY);
             }
         }

         for(int B = 1; A <= numWorldMusic; A++)
         {
             if(WorldMusic[B].Layer == LayerWld[A].Name)
             {
                   WorldMusic[B].Location.X += double(LayerWld[A].SpeedX);
                   WorldMusic[B].Location.Y += double(LayerWld[A].SpeedY);
             }
         }
    }
}
