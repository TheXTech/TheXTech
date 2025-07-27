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

#include "globals.h"
#include "msg_preprocessor.h"
#include "msg_macros.h"

void preProcessMessage(std::string &text, int playerWho)
{
    std::string ret;
    bool canCheckPlayers = playerWho > 0 && playerWho <= numPlayers;
    int macro_player = canCheckPlayers ? Player[playerWho].Character : -1;
    int macro_state = canCheckPlayers ? Player[playerWho].State : -1;

    msgMacroProcess(text, ret, macro_player, macro_state);

    text = ret;
}
