/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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


#include "pge_version.h" //Global Project version file

#ifndef THEXTECH_VERSION_H
#define THEXTECH_VERSION_H

#ifdef GIT_VERSION
#define V_BUILD_VER GIT_VERSION
#else
#define V_BUILD_VER "<empty>"
#endif

#ifdef GIT_BRANCH
#define V_BUILD_BRANCH GIT_BRANCH
#else
#define V_BUILD_BRANCH "<unknown>"
#endif

#ifdef THEXTECH_VERSION_1
#   define V_VF1 THEXTECH_VERSION_1
#else
#   define V_VF1 0
#endif

#ifdef THEXTECH_VERSION_2
#   define V_VF2 THEXTECH_VERSION_2
#else
#   define V_VF2 0
#endif

#ifdef THEXTECH_VERSION_3
#   define V_VF3 THEXTECH_VERSION_3
#else
#   define V_VF3 0
#endif

#ifdef THEXTECH_VERSION_4
#   define V_VF4 THEXTECH_VERSION_4
#else
#   define V_VF4 0
#endif

#ifdef THEXTECH_VERSION_REL
#   define V_FILE_RELEASE STR_VALUE(THEXTECH_VERSION_REL)
#else
#   define V_FILE_RELEASE "-unk" //"-alpha","-beta","-dev", or "" aka "release"
#endif

#define V_VF1_s STR_VALUE(V_VF1)
#define V_VF2_s STR_VALUE(V_VF2)
#define V_VF3_s STR_VALUE(V_VF3)
#define V_VF4_s STR_VALUE(V_VF4)
#if V_VF4 == 0
    #if V_VF3 == 0
        #define V_FILE_VERSION_NUM GEN_VERSION_NUMBER_2(V_VF1_s, V_VF2_s)
    #else
        #define V_FILE_VERSION_NUM GEN_VERSION_NUMBER_3(V_VF1_s, V_VF2_s, V_VF3_s)
    #endif
#else
    #define V_FILE_VERSION_NUM GEN_VERSION_NUMBER_4(V_VF1_s, V_VF2_s, V_VF3_s, V_VF4_s)
#endif

#define V_FILE_VERSION V_FILE_VERSION_NUM
//Version of this program
#define V_LATEST_STABLE V_FILE_VERSION_NUM V_FILE_RELEASE

#define V_FILE_DESC "TheXTech - a C++ source port of a vanilla Engine"

#define V_INTERNAL_NAME "thextech"

#ifdef _WIN32
    #define V_ORIGINAL_NAME "thextech.exe" // for Windows platforms
#else
    #define V_ORIGINAL_NAME "thextech" // for any other platforms
#endif

//Uncomment this for enable detal logging
//#define DEBUG

#endif
