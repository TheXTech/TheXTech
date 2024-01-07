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

#include <vector>
#include <regex>
#include "globals.h"

#include "msg_preprocessor.h"


static void s_splitString(std::vector<std::string>& out, const std::string& str, char delimiter)
{
    std::string::size_type beg = 0;
    std::string::size_type end = 0;

    out.clear();
    if(str.empty())
        return;

    do
    {
        end = str.find(delimiter, beg);
        if(end == std::string::npos)
            end = str.size();
        out.emplace_back(str.substr(beg, end - beg));
        beg = end + 1;
    }
    while(end < str.size() - 1);
}

void preProcessMessage(std::string& text, int playerWho)
{
    std::regex cond_if = std::regex("^#if *(\\w+\\(.*\\))$");
    std::regex cond_elif = std::regex("^#elif *(\\w+\\(.*\\))$");
    const std::string cond_endif = "#endif";
    const std::string cond_else = "#else";

    bool canCheckPlayers = playerWho > 0 && playerWho <= numPlayers;

    // fallback to P1
    if(!canCheckPlayers)
        playerWho = 1;

    std::regex reg_op_player = std::regex("^player\\((.*)\\)$");

    struct State
    {
        bool open = false;
        bool cond_true = false;
        bool skip_to_endif = false;
    };

    State st;
    std::string ret;
    std::string tmpS = text;
    std::vector<std::string> tmp;
    s_splitString(tmp, tmpS, '\n');

    if(tmp.size() < 1)
        return; // Nothing to do

    for(const auto &t : tmp)
    {
        std::smatch m_if;
        std::smatch m_elif;

        if((!st.open && std::regex_search(t, m_if, cond_if)) ||
            ( st.open && std::regex_search(t, m_elif, cond_elif)))
        {
            st.cond_true = false;
            if(st.open && st.skip_to_endif)
                continue;

            std::string cond = st.open ? m_elif[1].str() : m_if[1].str();
            std::smatch m_op_player;

            if(std::regex_search(cond, m_op_player, reg_op_player)) // check whaever player
            {
                st.open = true;
                std::string players = m_op_player[1].str();
                std::vector<std::string> nums;
                s_splitString(nums, players, ',');

                for(auto &i : nums)
                {
                    if(canCheckPlayers && std::atoi(i.c_str()) == Player[playerWho].Character)
                    {
                        st.cond_true = true;
                        st.skip_to_endif = true;
                        break;
                    }
                }
            }
            else // invalid line
            {
                if(!ret.empty())
                    ret.push_back('\n');
                ret.append(t);
            }
        }
        else if(st.open && t == cond_endif)
        {
            st.open = false;
            st.cond_true = false;
            st.skip_to_endif = false;
        }
        else if(st.open && t == cond_else)
        {
            // st.open = true; // keep same value
            st.cond_true = !st.skip_to_endif;
            if(st.open && st.skip_to_endif)
                continue;

            st.skip_to_endif = false;
        }
        else if(!st.open || st.cond_true) // ordinary line
        {
            if(!ret.empty())
                ret.push_back('\n');
            ret.append(t);
        }
    }

    text = ret;
}
