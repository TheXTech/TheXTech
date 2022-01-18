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

#pragma once
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
//    SpeedX As Single
    float SpeedX = 0.0f;
//    SpeedY As Single
    float SpeedY = 0.0f;
//End Type
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
    std::string music_file;

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
//    addSavedEvent As String
    std::string addSavedEvent;
//    RemoveSavedEvent As String
    std::string RemoveSavedEvent;
//    LayerSmoke As Boolean
    bool LayerSmoke = false;
//    Sound As Integer
    int Sound = 0;
//    Name As String
    std::string Name;
//    Text As String
    std::string Text;
//    HideLayer(0 To 20) As String
    std::vector<std::string> HideLayer;
//    ShowLayer(0 To 20) As String
    std::vector<std::string> ShowLayer;
//    ToggleLayer(0 To 20) As String
    std::vector<std::string> ToggleLayer;
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
    std::string TriggerEvent;
//    TriggerDelay As Double
    double TriggerDelay = 0.0;
//    Controls As Controls
    Controls_t Controls;
//    MoveLayer As String
    std::string MoveLayer;
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
const int maxLayers = 255; // 100
extern int numLayers;
extern RangeArr<Layer_t, 0, maxLayers> Layer;

//Public Events(0 To 100) As Events
const int maxEvents = 255; // 100
extern int numEvents;
extern RangeArr<Events_t, 0, maxEvents> Events;

//Public NewEvent(1 To 100) As String
extern RangeArr<std::string, 1, maxEvents> NewEvent;
//Public newEventDelay(1 To 100) As Integer
extern RangeArrI<int, 1, maxEvents, 0> newEventDelay;
//Public newEventNum As Integer
extern int newEventNum;

// Public Sub ShowLayer(LayerName As String, Optional NoEffect As Boolean = False) 'shows a layer
// shows a layer
void ShowLayer(const std::string &LayerName, bool NoEffect = false);

// Public Sub HideLayer(LayerName As String, Optional NoEffect As Boolean = False) 'hides a layer
// hides a layer
void HideLayer(const std::string &LayerName, bool NoEffect = false);

// Public Sub SetLayer(LayerName As String)
void SetLayer(const std::string &LayerName);

// Public Sub ProcEvent(EventName As String, Optional NoEffect As Boolean = False)
void ProcEvent(const std::string &EventName, bool NoEffect = false);

// Public Sub UpdateEvents()
void UpdateEvents();

// Public Sub UpdateLayers()
void UpdateLayers();

#endif // LAYERS_H
