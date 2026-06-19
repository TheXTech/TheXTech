/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2026 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef PGE_DELAY_H
#define PGE_DELAY_H

#ifdef __EMSCRIPTEN__
// IWYU pragma: begin_exports
#   include <emscripten.h>
#   define PGE_Delay(x) emscripten_sleep(x)
// IWYU pragma: end_exports
#elif defined(__3DS__)
// IWYU pragma: begin_exports
#   include <3ds.h>
#   define PGE_Delay(x) svcSleepThread((uint64_t)(x) * 1000000)
// IWYU pragma: end_exports
#elif defined(__WII__)
// IWYU pragma: begin_exports
extern "C" void udelay(unsigned us);
#   define PGE_Delay(x) udelay((x) * 1000)
// IWYU pragma: end_exports
#else
// IWYU pragma: begin_exports
#   include "sdl_proxy/sdl_timer.h"
#   define PGE_Delay(x) SDL_Delay(x)
// IWYU pragma: end_exports
#endif

#endif // PGE_DELAY_H
