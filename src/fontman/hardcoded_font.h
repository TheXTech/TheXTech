/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2024 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef HARDCODED_FONT_H
#define HARDCODED_FONT_H

#include <array>
#include <cstdint>

struct StdPicture;


// each byte is a row of the font, most significant bit is left
extern const std::array<uint8_t, 749> c_hardcoded_font_bytes;

static constexpr int c_hardcoded_font_w = 8;
static constexpr int c_hardcoded_font_h = 749; // 752 including padding
static constexpr int c_hardcoded_font_h_pow2 = 1024;

void LoadHardcodedFont(StdPicture& tx);

#endif // #ifndef HARDCODED_FONT_H
