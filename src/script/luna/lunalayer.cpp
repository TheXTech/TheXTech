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

#include "lunalayer.h"
#include "layers.h"

Layer_t *LayerF::Get(int layerIdx)
{
    if(layerIdx < 0 || layerIdx >= numLayers)
        return nullptr;
    return &Layer[layerIdx];
}

void LayerF::Stop(Layer_t *layer)
{
    layer->EffectStop = false;
    layer->SpeedX = 0.0001_nf;
    layer->SpeedY = 0.0001_nf;

    // NOTE from ds-sloth: why non-zero? probably, to make sure all objects get updated.
    // It would be better to set all objects (Blocks, BGOs, NPCs) in layer to speed 0.
    // keep it this way to preserve compatibility with existing Autocode content.
}

void LayerF::SetYSpeed(Layer_t *layer, num_t setY)
{
    layer->SpeedY = (setY == 0 ?  0.0001_nf : (numf_t)setY);
    layer->EffectStop = true;
}

void LayerF::SetXSpeed(Layer_t *layer, num_t setX)
{
    layer->SpeedX = (setX == 0 ?  0.0001_nf : (numf_t)setX);
    layer->EffectStop = true;
}
