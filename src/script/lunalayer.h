#pragma once
#ifndef LUNALAYER_H
#define LUNALAYER_H

struct Layer_t;

namespace LayerF
{

Layer_t *Get(int layerIdx);

// Stop a layer
void Stop(Layer_t *layer);

void SetYSpeed(Layer_t *layer, float setY);

void SetXSpeed(Layer_t *layer, float setX);

} // LayerF

#endif // LUNALAYER_H
