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

#ifndef LAYERS_H
#define LAYERS_H

#include <string>
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
    RangeArr<std::string, 0, 20> HideLayerWLD;
//    ShowLayer(0 To 20) As String
    RangeArr<std::string, 0, 20> ShowLayerWLD;
//    ToggleLayer(0 To 20) As String
    RangeArr<std::string, 0, 20> ToggleLayerWLD;
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
void ShowLayerWLD(std::string LayerName, bool NoEffect = false);

// Public Sub HideLayer(LayerName As String, Optional NoEffect As Boolean = False) 'hides a layer
// hides a layer
void HideLayerWLD(std::string LayerName, bool NoEffect = false);

// Public Sub ProcEvent(EventName As String, Optional NoEffect As Boolean = False)
void ProcEventWLD(std::string EventName, bool NoEffect = false);

// Public Sub UpdateEvents()
void UpdateEventsWLD();

// Public Sub UpdateLayers()
void UpdateLayersWLD();

#endif // LAYERS_WLD_H
