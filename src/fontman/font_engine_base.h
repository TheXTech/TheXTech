/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#pragma once
#ifndef FONT_ENGINE_BASE_H
#define FONT_ENGINE_BASE_H

#include <Graphics/size.h>
#include <string>

class BaseFontEngine
{
public:
    virtual ~BaseFontEngine();

    virtual PGE_Size textSize(std::string &text,
                              uint32_t max_line_lenght = 0,
                              bool cut = false, uint32_t fontSize = 14) = 0;

    virtual void printText(const std::string &text,
                           int32_t x, int32_t y,
                           float Red = 1.f, float Green = 1.f, float Blue = 1.f, float Alpha = 1.f,
                           uint32_t fontSize = 14) = 0;

    virtual bool isLoaded() = 0;

    virtual std::string getFontName() = 0;
};

#endif // FONT_ENGINE_BASE_H
