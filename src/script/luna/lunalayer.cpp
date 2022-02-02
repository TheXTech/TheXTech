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
