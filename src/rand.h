/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef RAND_H
#define RAND_H

/**
 * @brief Random number generator in float format, between 0.0f to 1.0f
 * @return random float value
 */
extern float fRand();

/**
 * @brief Random number generator in double format, between 0.0 to 1.0
 * @return random double value
 */
extern double dRand();

/**
 * @brief Random number generator in integer format, between 0.0 to RAND_MAX
 * @return random integer value
 */
extern int iRand();


#endif // RAND_H
