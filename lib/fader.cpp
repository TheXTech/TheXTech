/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#include "fader.h"
#include <stdlib.h>
#include <math.h>
#include <Utils/maths.h>

PGE_Fader::PGE_Fader()
{}

PGE_Fader::PGE_Fader(const PGE_Fader &f)
{
    this->operator=(f);
}

PGE_Fader::~PGE_Fader()
{}

PGE_Fader &PGE_Fader::operator=(const PGE_Fader &f)
{
    cur_ratio    = f.cur_ratio;
    target_ratio = f.target_ratio;
    fade_step    = f.fade_step;
    fadeSpeed    = f.fadeSpeed;
    manual_ticks = f.manual_ticks;
    return *this;
}

void PGE_Fader::setFade(int speed, double target, double step)
{
    fade_step    = fabs(step);
    target_ratio = target;
    fadeSpeed    = speed;
    manual_ticks = speed;
}

void PGE_Fader::setRatio(double ratio)
{
    cur_ratio    = ratio;
    target_ratio = ratio;
}

bool PGE_Fader::tickFader(double ticks)
{
    if(fadeSpeed < 1) return true; //Idling animation
    if(cur_ratio == target_ratio)
        return true;

    manual_ticks -= fabs(ticks);
    while(manual_ticks <= 0.0)
    {
        fadeStep();
        manual_ticks += fadeSpeed;
    }
    return (cur_ratio == target_ratio);
}

const double &PGE_Fader::fadeRatio()
{
    return cur_ratio;
}

bool PGE_Fader::isFading()
{
    return (cur_ratio != target_ratio);
}

bool PGE_Fader::isFull()
{
    return ((cur_ratio >= 1.0) && (!isFading()));
}

void PGE_Fader::setFull()
{
    cur_ratio    = 1.0;
    target_ratio = 1.0;
}

bool PGE_Fader::isNull()
{
    return ((cur_ratio <= 0.0) && (!isFading()));
}

void PGE_Fader::setNull()
{
    cur_ratio    = 0.0;
    target_ratio = 0.0;
}

int PGE_Fader::ticksLeft()
{
    return static_cast<int>(manual_ticks);
}

void PGE_Fader::fadeStep()
{
    if(cur_ratio == target_ratio)
        return;
    if(cur_ratio < target_ratio)
        cur_ratio += fade_step;
    else
        cur_ratio -= fade_step;

    if(cur_ratio > 1.0) cur_ratio = 1.0;
    else if(cur_ratio < 0.0) cur_ratio = 0.0;
}
