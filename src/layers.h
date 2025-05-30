/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#pragma once
#ifndef LAYERS_H
#define LAYERS_H

#include <string>
#include <vector>
#include <set>
#include <cstdint>

#include "range_arr.hpp"
#include "location.h"
#include "global_constants.h"
#include "control_types.h"

// also defined in "globals.h"
extern const std::string g_emptyString;

//Public Type Layer
struct Layer_t
{
//    EffectStop As Boolean
    bool EffectStop = false;
//    Hidden As Boolean
    bool Hidden = false;
    // Index into SavedLayers array, plus 1. (Normally 0, indicating a non-saved layer.)
    uint8_t SavedLayer = 0;
    // NEW: time until layer rejoins the main table
    uint8_t join_timer = 0;
//    Name As String
    std::string Name;
//    SpeedX As Single
    numf_t SpeedX = 0;
//    SpeedY As Single
    numf_t SpeedY = 0;

// NEW: track the actual applied SpeedX and SpeedY during the last frame. Used for vines.
    numf_t ApplySpeedX = 0;
    numf_t ApplySpeedY = 0;

//End Type
// NEW: track the objects belonging to the layer
    std::vector<vbint_t> blocks;
    std::vector<vbint_t> BGOs;
    std::set<int> NPCs;
    std::vector<vbint_t> warps;
    std::vector<vbint_t> waters;
// NEW: track the layer offset so we don't need to update the block/BGO trees
    num_t OffsetX = 0;
    num_t OffsetY = 0;
};

struct EventSection_t
{
    enum SetActions
    {
        LESet_Nothing = -1,
        LESet_ResetDefault = -2,
    };

    //! Change section borders if not (-1 - do nothing, -2 set default, any other values - set X position of left section boundary)
    IntegerLocation_t position;

    //! Set new Background ID in this section (-1 - do nothing, -2 - reset to defaint, >=0 - set background ID)
    vbint_t background_id = LESet_Nothing;

    //! Set new Music ID in this section (-1 - do nothing, -2 - reset to defaint, >=0 - set music ID)
    vbint_t music_id = LESet_Nothing;
    //! Set new Custom Music File path
    stringindex_t music_file = STRINGINDEX_NONE;

    //! Do override current autoscroll
    bool  autoscroll = false;
    //! X speed of autoscrool
    numf_t autoscroll_x = 0;
    //! Y speed of autoscrool
    numf_t autoscroll_y = 0;
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
    vbint_t Sound = 0;
//    Name As String
    std::string Name;
//    Text As String
    stringindex_t Text = STRINGINDEX_NONE;
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
    vbint_t TriggerDelay = 0;
//    Controls As Controls
    Controls_t Controls;
//    MoveLayer As String
    layerindex_t MoveLayer = LAYER_NONE;
//    SpeedX As Single
    numf_t SpeedX = 0;
//    SpeedY As Single
    numf_t SpeedY = 0;
//    AutoX As Single
    numf_t AutoX = 0;
//    AutoY As Single
    numf_t AutoY = 0;
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

//Public NewEvent(1 To 100) As String
extern RangeArrI<eventindex_t, 1, maxEvents, EVENT_NONE> NewEvent;
//Public newEventDelay(1 To 100) As Integer
extern RangeArrI<vbint_t, 1, maxEvents, 0> newEventDelay;
//! NEW: which player originally triggered each queued event
extern RangeArrI<uint8_t, 1, maxEvents, 0> newEventPlayer;
//Public newEventNum As Integer
extern int newEventNum;


// utilities for layerindex_t and eventindex_t

#define LAYER_USED_P_SWITCH_TITLE "Used P Switch"

extern layerindex_t LAYER_USED_P_SWITCH;

inline const std::string& GetL(layerindex_t index)
{
    if(index == LAYER_NONE)
    {
        return g_emptyString;
    }
    return Layer[index].Name;
}
inline const std::string& GetE(eventindex_t index)
{
    if(index == EVENT_NONE)
    {
        return g_emptyString;
    }
    return Events[index].Name;
}

// right now these are only used by the editor,
// so they are done using a linear search.
// they should be updated to use a hash map if
// they will be heavily used by autocode
layerindex_t FindLayer(const std::string& LayerName);
eventindex_t FindEvent(const std::string& EventName);


// Functions for layers

// New functions:

// swaps two layers and updates all references to them
bool SwapLayers(layerindex_t index_1, layerindex_t index_2);
// renames a layer
bool RenameLayer(layerindex_t index, const std::string& NewName);
// deletes a layer (and, optionally, everything in it)
bool DeleteLayer(layerindex_t index, bool killall);

// sets the speed of a layer
// EffectStop controls whether the layer will stop when a player has abnormal status
// Defective means that SMBX 1.3 was initially missing all code other than setting the speed
void SetLayerSpeed(layerindex_t index, num_t SpeedX, num_t SpeedY, bool EffectStop = true, bool Defective = false);

// Old functions:

// Public Sub ShowLayer(LayerName As String, Optional NoEffect As Boolean = False) 'shows a layer
// shows a layer
void ShowLayer(layerindex_t index, bool NoEffect = false);
// Public Sub HideLayer(LayerName As String, Optional NoEffect As Boolean = False) 'hides a layer
// hides a layer
void HideLayer(layerindex_t index, bool NoEffect = false);
// Public Sub SetLayer(LayerName As String)
// (unused)
void SetLayer(layerindex_t index);
// Public Sub UpdateLayers()
void UpdateLayers();


// Functions for events

// New functions:

// initializes an event
void InitializeEvent(Events_t& event);
// swaps two events and updates all references to them
bool SwapEvents(eventindex_t index_1, eventindex_t index_2);
// renames an event
bool RenameEvent(eventindex_t index, const std::string& NewName);
// deletes an event
bool DeleteEvent(eventindex_t index);

// EXTRA: Cancel awaiting event trigger
void CancelNewEvent(eventindex_t index);
// EXTRA: Check was any event got triggered?
bool EventWasTriggered(eventindex_t index);
// EXTRA: Clear up the tracklist
void ClearTriggeredEvents();

// Old functions:

// Public Sub ProcEvent(EventName As String, Optional NoEffect As Boolean = False)
void ProcEvent(eventindex_t, bool) = delete; // old signature
// NEW: added WhichPlayer, 0 by default, to indicate which player triggered the event
void ProcEvent(eventindex_t index, int WhichPlayer, bool NoEffect = false);

// If this returns an eventindex other than EVENT_NONE, then a pause has been initiated for a message
// As soon as the pause ends, the call must be re-made with the same arguments, but with resume set to true and index set to the returned eventindex
// The caller must also support an interrupt and restore routine.
eventindex_t ProcEvent_Safe(bool resume, eventindex_t index, int WhichPlayer, bool NoEffect = false);

// NEW: safe call that adds event to the end of the events queue for the current frame
void TriggerEvent(eventindex_t index, int WhichPlayer);

// Public Sub UpdateEvents()
bool UpdateEvents();


// functions to synchronize the layers of objects
// call these any time the layer of an object changes
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
