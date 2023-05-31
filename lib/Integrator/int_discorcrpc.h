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

#ifndef DISCORCRPC_H
#define DISCORCRPC_H

#include <stdint.h>
#include <string>

class DiscorcRPC
{
    int FrustrationLevel = 0;
    int SendPresence = 1;

    std::string episodeName;
    std::string levelName;
    std::string editorFile;

    std::string gameName;
    std::string iconName;

    void clearAllLabels();

public:
    DiscorcRPC();
    ~DiscorcRPC();

    void init();
    void quit();

    void setGameName(const std::string &game);
    void setEpisodeName(const std::string &ep);
    void setLevelName(const std::string &lev);
    void setEditorFile(const std::string &fil);
    void setIconName(const std::string &icon);

    void sync();

    void update();
    void clear();
};

#endif // DISCORCRPC_H
