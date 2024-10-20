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

#include <vector>

#include "fontman/hardcoded_font.h"

#include "core/render.h"

const std::array<uint8_t, 749> c_hardcoded_font_bytes = {0, 24, 24, 24, 24, 0, 24, 0, 0, 36, 36, 36, 0, 0, 0, 0, 0, 36, 126, 36, 126, 36, 0, 0, 0, 56, 108, 59, 110, 198, 123, 0, 98, 102, 12, 24, 48, 102, 198, 0, 0, 56, 108, 59, 110, 198, 123, 0, 0, 12, 12, 24, 0, 0, 0, 0, 0, 12, 24, 24, 24, 24, 12, 0, 0, 24, 12, 12, 12, 12, 24, 0, 0, 108, 124, 56, 124, 108, 0, 0, 0, 24, 24, 126, 24, 24, 0, 0, 0, 0, 0, 0, 12, 12, 24, 0, 0, 0, 0, 126, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 24, 0, 0, 12, 24, 48, 96, 64, 0, 0, 0, 60, 102, 110, 118, 102, 60, 0, 0, 24, 56, 24, 24, 24, 60, 0, 0, 60, 102, 6, 60, 96, 126, 0, 0, 60, 102, 12, 6, 102, 60, 0, 0, 96, 108, 108, 126, 12, 12, 0, 0, 126, 64, 124, 6, 102, 60, 0, 0, 60, 96, 124, 102, 102, 60, 0, 0, 126, 12, 12, 24, 24, 24, 0, 0, 60, 102, 60, 102, 102, 60, 0, 0, 60, 102, 102, 62, 6, 60, 0, 0, 24, 24, 0, 24, 24, 0, 0, 0, 24, 24, 0, 24, 24, 16, 0, 0, 12, 24, 48, 24, 12, 0, 0, 0, 62, 62, 0, 62, 62, 0, 0, 0, 48, 24, 12, 24, 48, 0, 0, 0, 60, 102, 6, 28, 0, 24, 0, 0, 60, 102, 110, 108, 96, 62, 0, 0, 60, 102, 102, 126, 102, 102, 0, 0, 124, 102, 124, 102, 102, 124, 0, 0, 60, 102, 96, 96, 102, 60, 0, 0, 124, 102, 102, 102, 102, 124, 0, 0, 126, 96, 120, 96, 96, 126, 0, 0, 126, 96, 120, 96, 96, 96, 0, 0, 60, 102, 96, 110, 102, 60, 0, 0, 102, 102, 126, 102, 102, 102, 0, 0, 60, 24, 24, 24, 24, 60, 0, 0, 62, 12, 12, 12, 108, 56, 0, 0, 102, 108, 120, 120, 108, 102, 0, 0, 96, 96, 96, 96, 96, 124, 0, 0, 99, 119, 127, 107, 99, 99, 0, 0, 102, 118, 126, 126, 110, 102, 0, 0, 60, 102, 102, 102, 102, 60, 0, 0, 124, 102, 102, 124, 96, 96, 0, 0, 60, 102, 102, 102, 108, 58, 0, 0, 124, 102, 102, 124, 102, 102, 0, 0, 60, 98, 60, 6, 102, 60, 0, 0, 126, 24, 24, 24, 24, 24, 0, 0, 102, 102, 102, 102, 102, 60, 0, 0, 102, 102, 102, 102, 60, 24, 0, 0, 99, 99, 107, 127, 119, 99, 0, 0, 102, 60, 24, 60, 102, 66, 0, 0, 102, 102, 60, 24, 24, 24, 0, 0, 126, 12, 24, 48, 96, 126, 0, 0, 28, 24, 24, 24, 24, 28, 0, 0, 96, 48, 24, 12, 4, 0, 0, 0, 28, 12, 12, 12, 12, 28, 0, 28, 54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 126, 0, 0, 48, 48, 24, 0, 0, 0, 0, 0, 0, 62, 102, 102, 102, 59, 0, 96, 96, 124, 102, 102, 102, 124, 0, 0, 0, 60, 102, 96, 102, 60, 0, 6, 6, 62, 102, 102, 102, 62, 0, 0, 0, 60, 102, 124, 96, 62, 0, 0, 14, 24, 60, 24, 24, 24, 0, 0, 0, 62, 102, 102, 62, 6, 60, 96, 96, 124, 102, 102, 102, 102, 0, 24, 0, 24, 24, 24, 24, 24, 0, 12, 0, 12, 12, 12, 12, 108, 56, 96, 96, 102, 124, 120, 108, 102, 0, 48, 48, 48, 48, 48, 48, 28, 0, 0, 0, 236, 214, 214, 214, 214, 0, 0, 0, 124, 102, 102, 102, 102, 0, 0, 0, 60, 102, 102, 102, 60, 0, 0, 0, 124, 102, 102, 124, 96, 96, 0, 0, 62, 102, 102, 62, 7, 6, 0, 0, 108, 118, 96, 96, 96, 0, 0, 0, 60, 96, 60, 6, 124, 0, 0, 48, 124, 48, 48, 54, 28, 0, 0, 0, 102, 102, 102, 102, 59, 0, 0, 0, 102, 102, 102, 60, 24, 0, 0, 0, 107, 107, 107, 107, 55, 0, 0, 0, 102, 126, 60, 126, 102, 0, 0, 0, 102, 102, 102, 62, 6, 60, 0, 0, 126, 12, 24, 48, 126, 0, 0, 12, 24, 112, 24, 24, 12, 0, 0, 24, 24, 24, 24, 24, 24, 0, 0, 48, 24, 14, 24, 24, 48, 0, 0, 0, 48, 90, 12};

#ifdef __16M__
void LoadHardcodedFont(StdPicture& tx)
{
    tx.reset();

    tx.inited = true;
    tx.l.path = "!F";
    tx.l.lazyLoaded = true;

    tx.w = c_hardcoded_font_w * 2;
    tx.h = c_hardcoded_font_h * 2;
    tx.l.flags = 0;
}
#else
void LoadHardcodedFont(StdPicture& tx)
{
    tx.reset();

    tx.w = c_hardcoded_font_w * 2;
    tx.h = c_hardcoded_font_h * 2;

    std::vector<uint32_t> image_data(c_hardcoded_font_bytes.size() * 8);

    auto i = c_hardcoded_font_bytes.cbegin();
    uint32_t* o = &image_data[0];

    while(i != c_hardcoded_font_bytes.cend())
    {
        uint8_t row = *(i++);
        *(o++) = (row & 0x80) ? 0xFFFFFFFF : 0;
        *(o++) = (row & 0x40) ? 0xFFFFFFFF : 0;
        *(o++) = (row & 0x20) ? 0xFFFFFFFF : 0;
        *(o++) = (row & 0x10) ? 0xFFFFFFFF : 0;
        *(o++) = (row & 0x08) ? 0xFFFFFFFF : 0;
        *(o++) = (row & 0x04) ? 0xFFFFFFFF : 0;
        *(o++) = (row & 0x02) ? 0xFFFFFFFF : 0;
        *(o++) = (row & 0x01) ? 0xFFFFFFFF : 0;
    }

    XRender::loadTexture(tx, c_hardcoded_font_w, c_hardcoded_font_h, (uint8_t*)image_data.data(), c_hardcoded_font_w * 4);
}
#endif
