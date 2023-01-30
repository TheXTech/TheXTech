/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef OUTRO_LOOP_H
#define OUTRO_LOOP_H

#include <string>

struct OutroContent
{
    std::string gameCredits;
    std::string engineCredits;

    std::string originalBy;
    std::string nameAndrewSpinks;

    std::string cppPortBy;
    std::string nameVitalyNovichkov;

    std::string d3dsPortBy;
    std::string wiiPortBy;
    std::string psVitaPortBy;

    std::string levelDesign;

    std::string customSprites;

    std::string specialThanks;
};

extern OutroContent g_outroScreen;

void initOutroContent();

#endif // OUTRO_LOOP_H
