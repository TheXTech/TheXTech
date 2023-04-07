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
#endif /* XTECH_TRANSLATE_EPISODE */

class TrTitleParser : public nlohmann::json_sax<nlohmann::json>
{
public:
    TrTitleParser() = default;
    TrTitleParser(const TrTitleParser&) = default;
    ~TrTitleParser() = default;

    const std::string m_wantedKey = "_episode_title";
    const std::string m_wantedKeyW = "_episode_world";
    std::string m_wantedWorld;
    std::string m_curKey;
    std::string m_curWorld;

    std::string *m_toWrite = nullptr;
    std::string m_outValue;
    bool m_hasValue = false;
    bool m_hasWorldTitle = false;

    enum Where
    {
        W_SKIP = 0,
        W_ROOT
    } m_where = W_SKIP;

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

    bool number_integer(number_integer_t)
    {
        return true;
    }

    bool number_unsigned(number_unsigned_t)
    {
        return true;
    }

    bool number_float(number_float_t, const string_t&)
    {
        return true;
    }

    bool string(string_t& val)
    {
        if(m_where == W_ROOT)
        {
            if(m_curKey == "_episode_title" && !val.empty())
            {
                m_outValue = val;
                m_hasValue = true;
                return true; // We found that we looked for
            }

            else if(m_curKey == "_episode_world" && !val.empty())
            {
                m_curWorld = val;
                m_hasWorldTitle = true;
                return true; // We found that we looked for
            }
        }

        if(m_hasWorldTitle && m_hasValue)
        {
            if(m_curWorld == m_wantedWorld)
                *m_toWrite = m_outValue;
            return false;
        }

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

        D_pLogDebug("JSON: Start Object (where=%d)", m_where);
        return true;
    }

    bool end_object()
    {
        if(m_where == W_SKIP)
            return true;

        D_pLogDebug("JSON: End Object (where=%d)", m_where);
        return true;
    }

    bool start_array(std::size_t)
    {
        return true;
    }

    bool end_array()
    {
        return true;
    }

    // called when an object key is parsed; value is passed and can be safely moved away
    bool key(string_t& val)
    {
        m_curKey = val;

        if(m_where == W_SKIP)
        {
            if(m_curKey == m_wantedKey || m_curKey == m_wantedKeyW)
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
