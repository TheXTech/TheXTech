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

#pragma once
#ifndef XTECH_TRANSLATE_EPISODE
#include <json/json.hpp>
#include <Logger/logger.h>
#include "globals.h"
#include "layers.h"
#endif /* XTECH_TRANSLATE_EPISODE */

class TrLevelParser : public nlohmann::json_sax<nlohmann::json>
{
public:
    TrLevelParser() = default;
    TrLevelParser(const TrLevelParser&) = default;
    ~TrLevelParser() = default;

    std::string m_wantedKey;
    std::string m_curKey;

    int m_outKey = -1;
    std::string m_outValue;

    enum Where
    {
        W_SKIP = 0,
        W_ROOT,
        W_LEVEL,
        W_NPC,    W_NPC_OBJ,
        W_EVENT,  W_EVENT_OBJ
    } m_where = W_SKIP;

    void flushData()
    {
        if(m_outKey < 0 || m_outValue.empty())
            return;

        D_pLogDebug("JSON: Written %d into %s", m_outKey, m_outValue.c_str());

        if(m_where == W_NPC_OBJ && m_outKey < numNPCs)
            SetS(NPC[m_outKey + 1].Text, m_outValue);
        else if(m_where == W_EVENT_OBJ && m_outKey < numEvents)
            SetS(Events[m_outKey].Text, m_outValue);

        m_outKey = -1;
        m_outValue.clear();
    }


    // called when null is parsed
    bool null()
    {
        return true;
    }

    // called when a boolean is parsed; value is passed
    bool boolean(bool)
    {
        return true;
    }

    bool number_integer(number_integer_t val)
    {
        if((m_where == W_NPC_OBJ || m_where == W_EVENT_OBJ) && m_curKey == "i")
            m_outKey = val;
        return true;
    }

    bool number_unsigned(number_unsigned_t val)
    {
        if((m_where == W_NPC_OBJ || m_where == W_EVENT_OBJ) && m_curKey == "i")
            m_outKey = (int)val;
        return true;
    }

    bool number_float(number_float_t, const string_t&)
    {
        return true;
    }

    bool string(string_t& val)
    {
        if(m_where == W_NPC_OBJ && m_curKey == "talk")
            m_outValue = val;
        else if(m_where == W_EVENT_OBJ && m_curKey == "msg")
            m_outValue = val;
        else if(m_where == W_LEVEL && m_curKey == "title")
            LevelName = m_outValue;

        return true;
    }

    bool binary(binary_t&)
    {
        return true;
    }

    // called when an object or array begins or ends, resp. The number of elements is passed (or -1 if not known)
    bool start_object(std::size_t)
    {
        if(m_where == W_SKIP)
            return true;

        if(m_where == W_ROOT)
            m_where = W_LEVEL;
        else if(m_where == W_NPC)
            m_where = W_NPC_OBJ;
        else if(m_where == W_EVENT)
            m_where = W_EVENT_OBJ;

        D_pLogDebug("JSON: Start Object (where=%d)", m_where);

        return true;
    }

    bool end_object()
    {
        if(m_where == W_SKIP)
            return true;

        D_pLogDebug("JSON: End Object (where=%d)", m_where);

        if(m_where == W_NPC_OBJ)
        {
            flushData();
            m_where = W_NPC;
        }
        else if(m_where == W_EVENT_OBJ)
        {
            flushData();
            m_where = W_EVENT;
        }
        else if(m_where == W_NPC)
            m_where = W_LEVEL;
        else if(m_where == W_EVENT)
            m_where = W_LEVEL;
        else if(m_where == W_LEVEL)
            return false; // All enough data has been taken

        return true;
    }

    bool start_array(std::size_t)
    {
        if(m_where == W_SKIP)
            return true;

        if(m_where == W_ROOT)
            m_where = W_LEVEL;
        else if(m_where == W_LEVEL)
        {
            if(m_curKey == "npc")
                m_where = W_NPC;
            else if(m_curKey == "events")
                m_where = W_EVENT;
        }

        D_pLogDebug("JSON: Start Array (where=%d)", m_where);

        return true;
    }

    bool end_array()
    {
        if(m_where == W_SKIP)
            return true;

        if(m_where == W_NPC || m_where == W_EVENT)
        {
            flushData();
            m_where = W_LEVEL;
            return true; // All enough data has been taken
        }

        return true;
    }

    // called when an object key is parsed; value is passed and can be safely moved away
    bool key(string_t& val)
    {
        m_curKey = val;

        if(m_where == W_SKIP)
        {
            if(Strings::endsWith(m_wantedKey, ".lvlx"))
                m_wantedKey.erase(m_wantedKey.end() - 1);

            if(Strings::endsWith(m_curKey, ".lvlx"))
                m_curKey.erase(m_curKey.end() - 1);

            if(m_curKey == m_wantedKey)
            {
                m_where = W_ROOT;
                D_pLogDebug("JSON: FOUND A KEY %s", val.c_str());
            }
            return true;
        }

        D_pLogDebug("JSON: Key=%s", val.c_str());

        return true;
    }

    bool parse_error(std::size_t position, const std::string& last_token, const nlohmann::detail::exception& ex)
    {
        pLogWarning("JSON parsing error: pos: %d, last token: %s, exception: %s",
                     position, last_token.c_str(), ex.what());
        return false;
    }
};
