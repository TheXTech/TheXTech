/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef SCREEN_PROGRESS_H
#define SCREEN_PROGRESS_H

#include <cstdint>
#include <string>

/**
 * \brief Updates a progress screen for a long main thread operation
 *
 * \param start_time: start time of the operation based on SDL_GetTicks() (required)
 * \param progress: estimated progress ratio between 0 and 1 (set to negative value if no estimate possible)
 * \param message:
 *
 * Will perform an events update and other actions required during main loop
 **/
void IndicateProgress(uint32_t start_time, double progress, const std::string& message);

#endif // SCREEN_PROGRESS_H
