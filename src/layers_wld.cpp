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
#include "layers.h"
#include "effect.h"
#include "collision.h"
#include "npc.h"
#include "sound.h"
#include "graphics.h"
#include "game_main.h"

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

void ShowLayerWLD(std::string LayerName, bool NoEffect)
{
    Location_t tempLocation;

    if(LayerName.empty())
        return;

    for(int A = 0; A <= maxLayers; A++)
    {
        if(equalCase(Layer[A].Name, LayerName))
            Layer[A].Hidden = true;
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

    for(int A = 0; A <= maxLayers; A++)
    {
        if(equalCase(Layer[A].Name, LayerName))
            Layer[A].Hidden = true;
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

    if(EventName.empty() || LevelEditor == true)
        return;

    for(A = 0; A <= maxEvents; A++)
    {
        if(equalCase(EventName, Events[A].Name))
        {
            for(B = 0; B <= maxSections; B++)
            {
                if(Events[A].Music[B] == -2)
                {
                    bgMusic[B] = bgMusicREAL[B];
                    if(B == Player[1].Section || (numPlayers == 2 && B == Player[2].Section))
                    {
                        StartMusic(B);
                    }
                }
                else if(Events[A].Music[B] != -1)
                {
                    bgMusic[B] = Events[A].Music[B];
                    if(B == Player[1].Section || (numPlayers == 2 && B == Player[2].Section))
                    {
                        StartMusic(B);
                    }
                }
                if(Events[A].Background[B] == -2)
                    Background2[B] = Background2REAL[B];
                else if(Events[A].Background[B] != -1)
                    Background2[B] = Events[A].Background[B];
                if(int(Events[A].level[B].X) == -2)
                    level[B] = LevelREAL[B];
                else if(int(Events[A].level[B].X) != -1)
                {
                    tempLevel = level[B];
                    level[B] = Events[A].level[B];
                    if(Events[A].AutoStart == false && Events[A].Name != "World - Start")
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
                            if(tempBool == false)
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

                    if(Events[A].Name != "Level - Start")
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
                            level[B] = Events[A].level[B];
                        }
                        else
                        {
                            qScreen = true;
                            qScreenX[1] = vScreenX[1];
                            qScreenY[1] = vScreenY[1];
                        }

                        overTime = 0;
                        GoalTime = SDL_GetTicks() + 1000;
                        fpsCount = 0;
                        cycleCount = 0;
                        gameTime = 0;
                        fpsTime = 0;
                    }
                }
            }

            for(B = 0; B <= 20; B++)
            {
                if(NoEffect == true)
                {
                    HideLayer(Events[A].HideLayer[B], NoEffect);
                    ShowLayer(Events[A].ShowLayer[B], NoEffect);
                }
                else
                {
                    HideLayer(Events[A].HideLayer[B], Events[A].LayerSmoke);
                    ShowLayer(Events[A].ShowLayer[B], Events[A].LayerSmoke);
                }

                if(!(Events[A].ToggleLayer[B] == ""))
                {
                    for(C = 0; C <= maxLayers; C++)
                    {
                        if(Layer[C].Name == Events[A].ToggleLayer[B])
                        {
                            if(Layer[C].Hidden == true)
                            {
                                ShowLayer(Layer[C].Name, Events[A].LayerSmoke);
                            }
                            else
                            {
                                HideLayer(Layer[C].Name, Events[A].LayerSmoke);
                            }
                        }
                    }
                }
            }

            if(!Events[A].MoveLayer.empty())
            {
                for(B = 0; B <= maxLayers; B++)
                {
                    if(Layer[B].Name == Events[A].MoveLayer)
                    {
                        Layer[B].EffectStop = true;
                        Layer[B].SpeedX = Events[A].SpeedX;
                        Layer[B].SpeedY = Events[A].SpeedY;
                        if(Layer[B].SpeedX == 0.f && Layer[B].SpeedY == 0.f)
                        {
                            // stop layer
                            Layer[B].EffectStop = false;
                            for(C = 1; C <= numBlock; C++)
                            {
                                if(Block[C].Layer == Layer[B].Name)
                                {
                                    Block[C].Location.SpeedX = double(Layer[B].SpeedX);
                                    Block[C].Location.SpeedY = double(Layer[B].SpeedY);
                                }
                            }
                            for(C = 1; C <= numNPCs; C++)
                            {
                                if(NPC[C].Layer == Layer[B].Name)
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
            }
            AutoX[Events[A].AutoSection] = Events[Events[A].AutoSection].AutoX;
            AutoY[Events[A].AutoSection] = Events[Events[A].AutoSection].AutoY;

            if(!Events[A].Text.empty())
            {
                MessageText = Events[A].Text;
                PauseGame(1);
                MessageText = "";
            }

            if(Events[A].Sound > 0)
            {
                SoundPause[Events[A].Sound] = 0;
                PlaySound(Events[A].Sound);
            }

            if(Events[A].EndGame == 1)
            {
                for(B = 0; B <= maxSections; B++)
                    bgMusic[B] = 0;
                StopMusic();
                LevelMacroCounter = 0;
                LevelMacro = 5;
            }

            ForcedControls = (Events[A].Controls.Down ||
                              Events[A].Controls.Left ||
                              Events[A].Controls.Right ||
                              Events[A].Controls.Start ||
                              Events[A].Controls.Up);

            ForcedControl = Events[A].Controls;

            tempBool = false;
            if(!Events[A].TriggerEvent.empty())
            {
                if(int(Events[A].TriggerDelay) == 0)
                {
                    for(B = 0; B <= maxEvents; B++)
                    {
                        if(Events[B].Name == Events[A].TriggerEvent)
                        {
                            if(Events[B].TriggerEvent == Events[A].Name)
                                tempBool = true;
                            break;
                        }
                    }
                    if(tempBool == false)
                    {
                        ProcEvent(Events[A].TriggerEvent);
                    }
                }
                else
                {
                    newEventNum++;
                    NewEvent[newEventNum] = Events[A].TriggerEvent;
                    newEventDelay[newEventNum] = Events[A].TriggerDelay * 6.5;
                }
            }
        }
    }
}

void UpdateEventsWLD()
{
    // this is for evetns that have a delay to call other events
    // this sub also updates the screen position for autoscroll levels
    int A = 0;

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
    for(A = 0; A <= maxSections; A++)
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

void UpdateLayersWLD()
{
    for(int A = 0; A <= maxLayers; A++)
    {
         for(int B = 1; A <= numTiles; A++)
         {
             if(Tile[B].Layer == Layer[A].Name)
             {
                 Tile[B].Location.X += double(Layer[A].SpeedX);
                 Tile[B].Location.Y += double(Layer[A].SpeedY);
             }
         }

         for(int B = 1; A <= numScenes; A++)
         {
             if(Scene[B].Layer == Layer[A].Name)
             {
                  Scene[B].Location.X += double(Layer[A].SpeedX);
                  Scene[B].Location.Y += double(Layer[A].SpeedY);
             }
         }

         for(int B = 1; A <= numWorldPaths; A++)
         {
             if(WorldPath[B].Layer == Layer[A].Name)
             {
                  WorldPath[B].Location.X += double(Layer[A].SpeedX);
                  WorldPath[B].Location.Y += double(Layer[A].SpeedY);
             }
         }

         for(int B = 1; A <= numWorldLevels; A++)
         {
             if(WorldLevel[B].Layer == Layer[A].Name)
             {
                  WorldLevel[B].Location.X += double(Layer[A].SpeedX);
                  WorldLevel[B].Location.Y += double(Layer[A].SpeedY);
             }
         }

         for(int B = 1; A <= numWorldMusic; A++)
         {
             if(WorldMusic[B].Layer == Layer[A].Name)
             {
                   WorldMusic[B].Location.X += double(Layer[A].SpeedX);
                   WorldMusic[B].Location.Y += double(Layer[A].SpeedY);
             }
         }
    }
}
