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

#ifndef XMESSAGE_H
#define XMESSAGE_H

#include <cstdint>

struct Controls_t;


// The purpose of these methods is to track the set of player-initiated events that may affect gameplay
namespace XMessage
{

enum class Type : uint8_t
{
    empty,
    press,
    release,
    menu_action,
};

struct Message
{
    Type type = Type::empty;
    uint8_t screen;
    uint8_t player;
    uint8_t message;

    inline Message() : screen(0), player(0) {}
    operator bool() const
    {
        return type != Type::empty;
    }
};

void Tick();

void PushMessage(Message message);
Message PopMessage();

void PushControls(int l_player_i, const Controls_t& controls);

} // namespace XMessage

#endif // #ifndef XMESSAGE_H
