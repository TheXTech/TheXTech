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

void ShowLayer(std::string LayerName, bool NoEffect)
{
    int A = 0;
    int B = 0;
    Location_t tempLocation;

    if(LayerName.empty())
        return;

    for(A = 0; A <= maxLayers; A++)
    {
        if(equalCase(Layer[A].Name, LayerName))
        {
            Layer[A].Hidden = false;
            if(Layer[A].Name == "Destroyed Blocks")
                Layer[A].Hidden = true;
            if(Layer[A].Name == "Spawned NPCs")
                Layer[A].Hidden = false;
        }
    }

    for(A = 1; A <= numNPCs; A++)
    {
        if(equalCase(NPC[A].Layer, LayerName))
        {
            if(NPC[A].Hidden == true)
            {
                if(NoEffect == false && NPC[A].Generator == false)
                {
                    tempLocation = NPC[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }

                if(LevelEditor == false)
                {
                    if(NPCWontHurt[NPC[A].Type] == false && NPCIsABonus[NPC[A].Type] == false && NPC[A].Active == true)
                    {
                        for(B = 1; B <= numPlayers; B++)
                        {
                            if(CheckCollision(Player[B].Location, NPC[A].Location) == true)
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
    }

    for(A = 1; A <= numBlock; A++)
    {
        if(equalCase(Block[A].Layer, LayerName))
        {
            // If Not (Block(A).DefaultType = 0 And Block(A).Layer = "Destroyed Blocks") Then
            if(Block[A].Hidden == true)
            {
                if(NoEffect == false && Block[A].Invis == false)
                {
                    tempLocation = Block[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Block[A].Hidden = false;
        }

        if(LayerName == "Destroyed Blocks")
        {
            if(Block[A].DefaultType > 0)
            {
                if(Block[A].Layer == LayerName)
                    Block[A].Layer = "Default";
                Block[A].Special = Block[A].DefaultSpecial;
                Block[A].Type = Block[A].DefaultType;
            }
        }
        // End If
    }

    int allBgos = numBackground + numLocked;
    for(A = 1; A <= allBgos; A++)
    {
        if(equalCase(Background[A].Layer, LayerName))
        {
            if(Background[A].Hidden == true)
            {
                if(NoEffect == false)
                {
                    tempLocation = Background[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Background[A].Hidden = false;
        }
    }

    for(A = 1; A <= numWarps; A++)
    {
        if(equalCase(Warp[A].Layer, LayerName))
            Warp[A].Hidden = false;
    }

    for(A = 1; A <= numWater; A++)
    {
        if(equalCase(Water[A].Layer, LayerName))
            Water[A].Hidden = false;
    }
}

void HideLayer(std::string LayerName, bool NoEffect)
{
    int A = 0;
    Location_t tempLocation;
    if(LayerName.empty())
        return;

    for(A = 0; A <= maxLayers; A++)
    {
        if(equalCase(Layer[A].Name, LayerName))
            Layer[A].Hidden = true;
    }

    for(A = 1; A <= numNPCs; A++)
    {
        if(equalCase(NPC[A].Layer, LayerName))
        {
            if(NPC[A].Hidden == false)
            {
                if(NoEffect == false && NPC[A].Generator == false)
                {
                    tempLocation = NPC[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            NPC[A].Hidden = true;
            if(NPC[A].Generator == false)
            {
                Deactivate(A);
            }
        }
    }

    for(A = 1; A <= numBlock; A++)
    {
        if(equalCase(Block[A].Layer, LayerName))
        {
            if(Block[A].Hidden == false)
            {
                if(NoEffect == false && Block[A].Invis == false)
                {
                    tempLocation = Block[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Block[A].Hidden = true;
        }
    }

    int allBgos = numBackground + numLocked;
    for(A = 1; A <= allBgos; A++)
    {
        if(equalCase(Background[A].Layer, LayerName))
        {
            if(Background[A].Hidden == false)
            {
                if(NoEffect == false)
                {
                    tempLocation = Background[A].Location;
                    tempLocation.X = tempLocation.X + tempLocation.Width / 2.0 - EffectWidth[10] / 2.0;
                    tempLocation.Y = tempLocation.Y + tempLocation.Height / 2.0 - EffectHeight[10] / 2.0;
                    NewEffect(10, tempLocation);
                }
            }
            Background[A].Hidden = true;
        }
    }

    for(A = 1; A <= numWarps; A++)
    {
        if(equalCase(Warp[A].Layer, LayerName))
            Warp[A].Hidden = true;
    }

    for(A = 1; A <= numWater; A++)
    {
        if(equalCase(Water[A].Layer, LayerName))
            Water[A].Hidden = true;
    }
}

void SetLayer(std::string /*LayerName*/)
{
    // Unused
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

    if(EventName.empty() || LevelEditor == true)
        return;

    for(A = 0; A <= numEvents; A++)
    {
        if(equalCase(EventName, Events[A].Name))
        {
            for(B = 0; B <= numSections; B++)
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
                    if(Events[A].AutoStart == false && Events[A].Name != "Level - Start")
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

            for(auto &l : Events[A].HideLayer)
            {
                HideLayer(l, NoEffect ? true : Events[A].LayerSmoke);
            }

            for(auto &l : Events[A].ShowLayer)
            {
                ShowLayer(l, NoEffect ? true : Events[A].LayerSmoke);
            }

            for(auto &l : Events[A].ToggleLayer)
            {
                if(l.empty())
                    continue;

                for(C = 0; C <= numLayers; C++)
                {
                    if(Layer[C].Name == l)
                    {
                        if(Layer[C].Hidden)
                            ShowLayer(Layer[C].Name, Events[A].LayerSmoke);
                        else
                            HideLayer(Layer[C].Name, Events[A].LayerSmoke);
                    }
                }
            }

#if 0 // Obsolete, replaced with a code above
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
                            if(Layer[C].Hidden)
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
#endif

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
                for(B = 0; B <= numSections; B++)
                    bgMusic[B] = 0;
                StopMusic();
                LevelMacroCounter = 0;
                LevelMacro = 5;
            }

            ForcedControls = (Events[A].Controls.AltJump ||
                              Events[A].Controls.AltRun ||
                              Events[A].Controls.Down ||
                              Events[A].Controls.Drop ||
                              Events[A].Controls.Jump ||
                              Events[A].Controls.Left ||
                              Events[A].Controls.Right ||
                              Events[A].Controls.Run ||
                              Events[A].Controls.Start ||
                              Events[A].Controls.Up);

            ForcedControl = Events[A].Controls;

            tempBool = false;
            if(!Events[A].TriggerEvent.empty())
            {
                if(std::round(Events[A].TriggerDelay) == 0.0)
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

                    if(!tempBool)
                        ProcEvent(Events[A].TriggerEvent);
                }
                else
                {
                    newEventNum++;
                    NewEvent[newEventNum] = Events[A].TriggerEvent;
                    newEventDelay[newEventNum] = vb6Round(Events[A].TriggerDelay * 6.5);
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
    if(FreezeNPCs == true)
        return;

    if(GameMenu == false)
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
            {
                for(A = 0; A <= maxLayers; A++)
                {
                    if(Layer[A].Name != "" && (Layer[A].SpeedX != 0.f || Layer[A].SpeedY != 0.f) && Layer[A].EffectStop)
                    {
                        for(C = 1; C <= numBlock; C++)
                        {
                            if(Block[C].Layer == Layer[A].Name)
                            {
                                Block[C].Location.SpeedX = 0;
                                Block[C].Location.SpeedY = 0;
                            }
                        }
                    }
                }
                FreezeLayers = true;
            }
        }
    }

    for(A = 0; A <= maxLayers; A++)
    {
        if(FreezeNPCs == true)
        {
            if(Layer[A].Name != "" && (Layer[A].SpeedX != 0.f || Layer[A].SpeedY != 0.f))
            {
                for(B = 1; B <= numBlock; B++)
                {
                    if(Block[B].Layer == Layer[A].Name)
                    {
                        Block[B].Location.SpeedX = 0;
                        Block[B].Location.SpeedY = 0;
                    }
                }
            }
        }
        else
        {
            if(!Layer[A].Name.empty() && (Layer[A].SpeedX != 0.f || Layer[A].SpeedY != 0.f) &&
               !(FreezeLayers && Layer[A].EffectStop))
            {
                for(B = 1; B <= numBlock; B++)
                {
                    if(Block[B].Layer == Layer[A].Name)
                    {
                        if(Layer[A].SpeedX != 0.f)
                        {
                            if(BlocksSorted == true)
                            {
                                for(C = (int)(-FLBlocks); C <= FLBlocks; C++)
                                {
                                    FirstBlock[C] = 1;
                                    LastBlock[C] = numBlock;
                                }
                                BlocksSorted = false;
                            }
                        }
                        Block[B].Location.X += double(Layer[A].SpeedX);
                        Block[B].Location.Y += double(Layer[A].SpeedY);
                        Block[B].Location.SpeedX = double(Layer[A].SpeedX);
                        Block[B].Location.SpeedY = double(Layer[A].SpeedY);
                    }
                }

                int allBGOs = numBackground + numLocked;
                for(B = 1; B <= allBGOs; B++)
                {
                    if(Background[B].Layer == Layer[A].Name)
                    {
                        Background[B].Location.X += double(Layer[A].SpeedX);
                        Background[B].Location.Y += double(Layer[A].SpeedY);
                    }
                }

                for(B = 1; B <= numWater; B++)
                {
                    if(Water[B].Layer == Layer[A].Name)
                    {
                        Water[B].Location.X += double(Layer[A].SpeedX);
                        Water[B].Location.Y += double(Layer[A].SpeedY);
                    }
                }

                for(B = 1; B <= numNPCs; B++)
                {
                    if(NPC[B].Layer == Layer[A].Name)
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
                                else if(NPC[B].Type == 52 && fEqual(NPC[B].Direction, -1))
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
                }

                for(B = 1; B <= numWarps; B++)
                {
                    if(Warp[B].Layer == Layer[A].Name)
                    {
                        Warp[B].Entrance.X += double(Layer[A].SpeedX);
                        Warp[B].Entrance.Y += double(Layer[A].SpeedY);
                        Warp[B].Exit.X += double(Layer[A].SpeedX);
                        Warp[B].Exit.Y += double(Layer[A].SpeedY);
                    }
                }
            }
        }
    }
}
