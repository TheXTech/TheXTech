/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
