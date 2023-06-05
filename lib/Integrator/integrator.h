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

#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include <string>

namespace Integrator
{
#ifdef ENABLE_XTECH_INTEGRATOR

void initIntegrations();
void quitIntegrations();

void setGameName(const std::string &gameName, const std::string &iconName);

void setEpisodeName(const std::string &episodeName);
void clearEpisodeName();

void setLevelName(const std::string &levelName);
void clearLevelName();

void setEditorFile(const std::string &editorFile);
void clearEditorFile();

void sync();

#else

// Empty calls that does nothing

static inline void initIntegrations() {}
static inline void quitIntegrations() {}

static inline void setGameName(const std::string &, const std::string &) {}

static inline void setEpisodeName(const std::string &) {}
static inline void clearEpisodeName() {}

static inline void setLevelName(const std::string &) {}
static inline void clearLevelName() {}

static inline void setEditorFile(const std::string &) {}
static inline void clearEditorFile() {}

static inline void sync() {}

#endif
}

#endif // INTEGRATOR_H
