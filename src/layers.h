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

#ifndef LAYERS_H
#define LAYERS_H

#include <string>
#include <vector>
#include "range_arr.hpp"
#include "location.h"
#include "global_constants.h"
#include "controls.h"

//Public Type Layer
struct Layer_t
{
//    EffectStop As Boolean
    bool EffectStop = false;
//    Name As String
    std::string Name;
//    Hidden As Boolean
    bool Hidden = false;
    double OffsetX = 0.0;
    double OffsetY = 0.0;
//    SpeedX As Single
    float SpeedX = 0.0f;
//    SpeedY As Single
    float SpeedY = 0.0f;
//End Type
    std::set<int> blocks;
    std::set<int> BGOs;
    std::set<int> NPCs;
    std::set<int> warps;
    std::set<int> waters;
};

struct EventSection_t
{
    enum SetActions
    {
        LESet_Nothing = -1,
        LESet_ResetDefault = -2,
    };

    //! Set new Music ID in this section (-1 - do nothing, -2 - reset to defaint, >=0 - set music ID)
    int music_id = LESet_Nothing;
    //! Set new Custom Music File path
    stringindex_t music_file = STRINGINDEX_NONE;

    //! Set new Background ID in this section (-1 - do nothing, -2 - reset to defaint, >=0 - set background ID)
    int background_id = LESet_Nothing;

    //! Change section borders if not (-1 - do nothing, -2 set default, any other values - set X position of left section boundary)
    Location_t position;

    //! Do override current autoscroll
    bool  autoscroll = false;
    //! X speed of autoscrool
    float autoscroll_x = 0.0;
    //! Y speed of autoscrool
    float autoscroll_y = 0.0;
};


//Public Type Events
struct Events_t
{
    // never implemented by Redigit; we can add back in later
//    addSavedEvent As String
    // std::string addSavedEvent;
//    RemoveSavedEvent As String
    // std::string RemoveSavedEvent;
//    LayerSmoke As Boolean
    bool LayerSmoke = false;
//    Sound As Integer
    int Sound = 0;
//    Name As String
    std::string Name;
//    Text As String
    std::string Text;
//    HideLayer(0 To 20) As String
    std::vector<layerindex_t> HideLayer;
//    ShowLayer(0 To 20) As String
    std::vector<layerindex_t> ShowLayer;
//    ToggleLayer(0 To 20) As String
    std::vector<layerindex_t> ToggleLayer;
//    Music(0 To maxSections) As Integer
//    RangeArrI<int, 0, maxSections, 0> Music;
//    Background(0 To maxSections) As Integer
//    RangeArrI<int, 0, maxSections, 0> Background;
//    level(0 To maxSections) As Location
//    RangeArr<Location_t, 0, maxSections> level;
// EXTRA: Override per-section settings
    RangeArr<EventSection_t, 0, maxSections> section;
//    EndGame As Integer
    int EndGame = 0;
//    TriggerEvent As String
    eventindex_t TriggerEvent = EVENT_NONE;
//    TriggerDelay As Double
    double TriggerDelay = 0.0;
//    Controls As Controls
    Controls_t Controls;
//    MoveLayer As String
    layerindex_t MoveLayer = LAYER_NONE;
//    SpeedX As Single
    float SpeedX = 0.0f;
//    SpeedY As Single
    float SpeedY = 0.0f;
//    AutoX As Single
    float AutoX = 0.0f;
//    AutoY As Single
    float AutoY = 0.0f;
//    AutoSection As Integer
    int AutoSection = 0;
//    AutoStart As Boolean
    bool AutoStart = false;
//End Type
};

//Public Layer(0 To 100) As Layer
#ifdef LOW_MEM
const int maxLayers = 100; // 100
#else
const int maxLayers = 254; // 100
#endif
extern int numLayers;
extern RangeArr<Layer_t, 0, maxLayers> Layer;

//Public Events(0 To 100) As Events
#ifdef LOW_MEM
const int maxEvents = 100; // 100
#else
const int maxEvents = 254; // 100
#endif
extern int numEvents;
extern RangeArr<Events_t, 0, maxEvents> Events;

// utilities for layerindex_t and eventindex_t
extern layerindex_t LAYER_USED_P_SWITCH;

inline std::string GetL(layerindex_t index)
{
    if(index == LAYER_NONE)
    {
        return "";
    }
    return Layer[index].Name;
}
inline std::string GetE(eventindex_t index)
{
    if(index == EVENT_NONE)
    {
        return "";
    }
    return Events[index].Name;
}

//Public NewEvent(1 To 100) As String
extern RangeArrI<eventindex_t, 1, maxEvents, EVENT_NONE> NewEvent;
//Public newEventDelay(1 To 100) As Integer
extern RangeArrI<int, 1, maxEvents, 0> newEventDelay;
//Public newEventNum As Integer
extern int newEventNum;

// Public Sub ShowLayer(LayerName As String, Optional NoEffect As Boolean = False) 'shows a layer
// shows a layer
void ShowLayer(layerindex_t index, bool NoEffect = false);

// Public Sub HideLayer(LayerName As String, Optional NoEffect As Boolean = False) 'hides a layer
// hides a layer
void HideLayer(layerindex_t index, bool NoEffect = false);

// Public Sub SetLayer(LayerName As String)
void SetLayer(layerindex_t index);

layerindex_t FindLayer(const std::string LayerName);

bool SwapLayers(layerindex_t index_1, layerindex_t index_2);

bool RenameLayer(layerindex_t index, const std::string NewName);

bool DeleteLayer(layerindex_t index, bool killall);

void InitializeEvent(Events_t& event);

eventindex_t FindEvent(const std::string EventName);

bool SwapEvents(eventindex_t index_1, eventindex_t index_2);

bool RenameEvent(eventindex_t index, const std::string NewName);

bool DeleteEvent(eventindex_t index);

// Public Sub ProcEvent(EventName As String, Optional NoEffect As Boolean = False)
void ProcEvent(eventindex_t index, bool NoEffect = false);

// Public Sub UpdateEvents()
void UpdateEvents();

// Public Sub UpdateLayers()
void UpdateLayers();

void syncLayersTrees_AllBlocks();
void syncLayersTrees_Block(int block);
void syncLayersTrees_Block_SetHidden(int block); // set block hidden based on layer

void syncLayers_AllNPCs();
void syncLayers_NPC(int npc);

void syncLayers_AllBGOs();
void syncLayers_BGO(int bgo);

void syncLayers_Warp(int warp);

void syncLayers_Water(int water);

#endif // LAYERS_H
