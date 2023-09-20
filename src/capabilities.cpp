/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "../version.h"
#include "capabilities.h"


const char *const g_capabilities =
    "{"
        "\"version\" : \"" V_LATEST_STABLE "\","
        "\"ipc-protocols\" : ["
            "\"moondust-stdinout\","
            "\"--END--\""
        "],"
        "\"arguments\" : ["
            "\"c\","
            "\"u\","
            "\"game-dirname\","
            "\"frameskip\","
            "\"no-frameskip\","
            "\"no-sound\","
            "\"never-pause\","
            "\"bg-input\","
            "\"vsync\","
            "\"render\","
            "\"leveltest\","
            "\"num-players\","
            "\"start-warp\","
            "\"battle\","
            "\"player1\","
            "\"player2\","
            "\"god-mode\","
            "\"grab-all\","
            "\"show-fps\","
            "\"max-fps\","
            "\"magic-hand\","
            "\"editor\","
            "\"interprocessing\","
            "\"compat-level\","
            "\"speed-run-mode\","
            "\"speed-run-semitransparent\","
            "\"speed-run-blink-mode\","
            "\"show-controls\","
            "\"show-battery-status\","
            "\"save-slot\","
            "\"export-lang\","
            "\"lang\","
            "\"verbose\","
            "\"levelpath\","
            "\"--END--\""
        "],"
        "\"renders\" : ["
            "\"sw\","
            "\"hw\","
            "\"vsync\","
            "\"sdl\","
#ifdef THEXTECH_BUILD_GL_DESKTOP_MODERN
            "\"opengl\","
#endif
#ifdef THEXTECH_BUILD_GL_ES_MODERN
            "\"opengles\","
#endif
#ifdef THEXTECH_BUILD_GL_DESKTOP_LEGACY
            "\"opengl11\","
#endif
#ifdef THEXTECH_BUILD_GL_ES_LEGACY
            "\"opengles11\","
#endif
            "\"--END--\""
        "],"
        "\"features\" : ["
            "\"stars-number\","
            "\"editor\","
            "\"test-level-file\","
            "\"test-level-ipc\","
            "\"test-world-file\","
            "\"vsync-flag\","
            "\"--END--\""
        "]"
    "}";
