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

#ifndef EXTRAS_H
#define EXTRAS_H

#ifdef THEXTECH_IOS
#   ifdef __cplusplus
extern "C"
{
#   endif

/**
 * @brief Attempt to gracefully quit the app
 * @param ret Exit code same as returned via Main
 */
void ios_quit(int ret);

double ios_get_screen_diagonal(double *ow, double *oh);

int ios_get_cut_off_size(void);

#   ifdef __cplusplus
}
#   endif
#endif

#ifdef THEXTECH_TVOS
#   ifdef __cplusplus
extern "C"
{
#   endif

void tvos_quit(int ret);

#   ifdef __cplusplus
}
#   endif
#endif

#endif // EXTRAS_H
