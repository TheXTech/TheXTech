#include "lunalayer.h"
#include "layers.h"

Layer_t *LayerF::Get(int layerIdx)
{
    if(layerIdx < 0 || layerIdx > numLayers)
        return nullptr;
    return &Layer[layerIdx];
}

void LayerF::Stop(Layer_t *layer)
{
    layer->EffectStop = false;
    layer->SpeedX = 0.0001f;
    layer->SpeedY = 0.0001f;
}

void LayerF::SetYSpeed(Layer_t *layer, float setY)
{
    setY = (setY == 0 ?  0.0001f : setY);
    layer->SpeedY = setY;
    layer->EffectStop = true;
}

void LayerF::SetXSpeed(Layer_t *layer, float setX)
{
    setX = (setX == 0 ?  0.0001f : setX);
    layer->SpeedX = setX;
    layer->EffectStop = true;
}
