/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef XERROR_H
#define XERROR_H

#if defined(__GNUC__) || defined(__clang__)
#   define XERROR_NO_RETURN_ATTR __attribute__((__noreturn__))
#else
#   define XERROR_NO_RETURN_ATTR
#endif

/*!
 * \brief Trigger the fatal error caused by range array index out of range
 * \param r_low Lowest index
 * \param r_high Highest index
 * \param index Actual index
 * \param file Just pass a __FILE__
 * \param func Just pass a __FUNCTION__
 * \param line_number Just pass a __LINE__
 */
void fatal_assert_rangearr_real(long r_low, long r_high, long index, const char *file, const char *func, int line_number) XERROR_NO_RETURN_ATTR;

#define fatal_assert_rangearr(low, high, index) fatal_assert_rangearr_real(low, high, index, __FILE__, __FUNCTION__, __LINE__)

#endif // XERROR_H
