#ifndef LAYERS_H
#define LAYERS_H

#include <string>

// Public Sub ShowLayer(LayerName As String, Optional NoEffect As Boolean = False) 'shows a layer
// shows a layer
void ShowLayer(std::string LayerName, bool NoEffect = false);
// Public Sub HideLayer(LayerName As String, Optional NoEffect As Boolean = False) 'hides a layer
// hides a layer
void HideLayer(std::string LayerName, bool NoEffect = false);
// Public Sub SetLayer(LayerName As String)
void SetLayer(std::string LayerName);
// Public Sub ProcEvent(EventName As String, Optional NoEffect As Boolean = False)
void ProcEvent(std::string EventName, bool NoEffect = false);
// Public Sub UpdateEvents()
void UpdateEvents();
// Public Sub UpdateLayers()
void UpdateLayers();


#endif // LAYERS_H
