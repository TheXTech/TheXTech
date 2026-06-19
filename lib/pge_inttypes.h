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

#pragma once
#ifndef PGE_INTTYPES_H
#define PGE_INTTYPES_H

// IWYU pragma: begin_exports
#include <stdint.h>
#include <inttypes.h>
// IWYU pragma: end_exports

#if defined(__3DS__) || defined(__PSP__)
#   define PGE_INTTYPES_INT32_IS_LONG
#endif

#if INTPTR_MAX == INT32_MAX
#   define PGE_INTTYPES_INT64_IS_LONG_LONG
#endif

/* Workarounds */
#ifndef PRId32
#   ifdef PGE_INTTYPES_INT32_IS_LONG
#       define PRId32 "ld"
#   else
#       define PRId32 "d"
#   endif
#endif

#ifndef PRIu32
#   ifdef PGE_INTTYPES_INT32_IS_LONG
#       define PRIu32 "lu"
#   else
#       define PRIu32 "u"
#   endif
#endif

#ifndef PRIX32
#   ifdef PGE_INTTYPES_INT32_IS_LONG
#       define PRIX32 "lX"
#   else
#       define PRIX32 "X"
#   endif
#endif

#ifndef PRId64
#   ifdef PGE_INTTYPES_INT64_IS_LONG_LONG
#       define PRId64 "lld"
#   else
#       define PRId64 "ld"
#   endif
#endif

#ifndef PRIu64
#   ifdef PGE_INTTYPES_INT64_IS_LONG_LONG
#       define PRIu64 "llu"
#   else
#       define PRIu64 "lu"
#   endif
#endif

#undef PGE_INTTYPES_INT32_IS_LONG
#undef PGE_INTTYPES_INT64_IS_LONG_LONG

#endif // PGE_INTTYPES_H
