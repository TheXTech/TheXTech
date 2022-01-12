#pragma once
#ifndef RENDEROP_EFFECT_H
#define RENDEROP_EFFECT_H

#include "renderop.h"

typedef uint32_t COLORREF;

enum RENDER_EFFECT
{
    RNDEFF_ScreenGlow,
    RNDEFF_Flip
};

enum BLEND_TYPE
{
    BLEND_Additive,
    BLEND_Subtractive
};

enum FLIP_TYPE
{
    FLIP_TYPE_NONE = 0x0,
    FLIP_TYPE_X = 0x1,
    FLIP_TYPE_Y = 0x2,
    FLIP_TYPE_XY = 0x3,
};

class RenderEffectOp : public RenderOp
{
public:
    RenderEffectOp();
    RenderEffectOp(RENDER_EFFECT effect, BLEND_TYPE blend, COLORREF col, int intensity)
    {
        effect_type = effect;
        blend_type = blend;
        color = col;
        this->intensity = intensity;
        flip_type = FLIP_TYPE_NONE;
    }

    void Draw(Renderer *renderer);

    // Effects //
    void ScreenGlow(Renderer *renderer);
    void Flip(Renderer *renderer);

    // Members //
    RENDER_EFFECT effect_type;
    BLEND_TYPE blend_type;
    COLORREF color;
    int intensity;
    FLIP_TYPE flip_type;
};


#endif // RENDEROP_EFFECT_H
