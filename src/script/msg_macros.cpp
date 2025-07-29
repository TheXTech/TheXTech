/*
 * Moondust, a free game engine and development kit for platform game making
 * Copyright (c) 2014-2025 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#include <cstring>
#include <string>
#include <vector>
#include "msg_macros.h"

#ifndef MOONDUST_UNIT_TEST
static
#endif
bool msgMacroParseTokens(const std::string &line, std::vector<std::string> &tokens)
{
    size_t b, s;
    std::string buf;
    const char *cur = line.c_str();

    tokens.clear();

    if(line.empty() || *cur != '#')
        return false; // No key words

    b = 1;
    ++cur;

    while(*cur != '\0')
    {
        bool found = false;
        bool charToken = false;
        int skip = 0;

        for(s = 0; *cur != '\0'; ++s, ++cur)
        {
            switch(*cur)
            {
            case ' ':
                found = true;
                skip = 1;
                break;

            case '(':
            case ')':
            case ',':
                charToken = true;
                found = true;
                skip = 1;
                break;
            }

            if(found)
                break;
        }

        if(s > 0)
        {
            buf.assign(line.c_str() + b, s);
            tokens.push_back(buf);
        }

        b += s + skip;

        if(*cur != '\0' && charToken)
        {
            buf.assign(cur, 1);
            tokens.push_back(buf);
        }

        cur += skip;
    }

    return !tokens.empty();
}

enum CondCmd
{
    CondCmd_Unknown = -1,
    // Regular "if"
    CondCmd_If = 0,
    // Regular "elif"
    CondCmd_Elif,
    // Regular "endif"
    CondCmd_Else,
    // Regular "else"
    CondCmd_Endif,
    // "if" without line breaking, space will be appended
    CondCmd_IW_If,
    // "if" without line breaking, nothing will be appended
    CondCmd_IN_If,
    // "endif" without line breaking, space will be appended
    CondCmd_IW_Endif,
    // "endif" without line breaking, nothing will be appended
    CondCmd_IN_Endif,
};

static CondCmd enumerateCmd(const std::string &token)
{
    CondCmd ret = CondCmd_Unknown;

    if(token == "if")
        ret = CondCmd_If;
    else if(token == "elif")
        ret = CondCmd_Elif;
    else if(token == "else")
        ret = CondCmd_Else;
    else if(token == "endif")
        ret = CondCmd_Endif;
    else if(token == "if_iw")
        ret = CondCmd_IW_If;
    else if(token == "if_in")
        ret = CondCmd_IN_If;
    else if(token == "endif_iw")
        ret = CondCmd_IW_Endif;
    else if(token == "endif_in")
        ret = CondCmd_IN_Endif;
    else
        ret = CondCmd_Unknown;

    return ret;
}

enum CondFunc
{
    CondFunc_Unknown = -1,
    CondFunc_Player = 0,
    CondFunc_State,
};

static CondFunc enumerateFunc(const std::string &token)
{
    CondFunc ret = CondFunc_Unknown;

    if(token == "player")
        ret = CondFunc_Player;
    else if(token == "state")
        ret = CondFunc_State;
    else
        ret = CondFunc_Unknown;

    return ret;
}

static bool isUNum(const std::string &n)
{
    for(auto c : n)
    {
        if(!std::isdigit(c))
            return false;
    }

    return true;
}

static void toLower(std::string &n)
{
    for(auto &c : n)
        c = std::tolower(c);
}

static bool check_cond_if(const std::vector<std::string> &tokens, bool needElif, CondCmd &ret_cmd, CondFunc &ret_func, std::vector<int> &values)
{
    bool wantComma = false;
    std::string cond, func;

    values.clear();

    if(tokens.empty())
        return false;

    cond = tokens[0];
    toLower(cond);

    ret_cmd = enumerateCmd(cond);

    if(!needElif && ret_cmd != CondCmd_If && ret_cmd != CondCmd_IW_If && ret_cmd != CondCmd_IN_If)
        return false;

    if(needElif && ret_cmd != CondCmd_Elif)
        return false;

    if(tokens.size() < 5)
        return false;

    func = tokens[1];
    toLower(func);

    ret_func = enumerateFunc(func);

    for(size_t i = 3; i < tokens.size(); ++i)
    {
        auto &s = tokens[i];

        if(wantComma)
        {
            if(s != "," && s != ")")
                return false;

            if(s == ")") // Arguments parsing done
            {
                if(i != tokens.size() - 1)
                    return false;

                break;
            }

            wantComma = false;
        }
        else
        {
            if(!isUNum(s))
                return false;

            wantComma = true;
            values.push_back(std::atoi(s.c_str()));
        }
    }

    return !values.empty();
}

static bool check_cond_else(const std::vector<std::string> &tokens, CondCmd &ret_cmd)
{
    std::string cond;

    if(tokens.empty() || tokens.size() > 1)
        return false;

    cond = tokens[0];
    toLower(cond);

    ret_cmd = enumerateCmd(cond);

    return ret_cmd == CondCmd_Else;
}

static bool check_cond_endif(const std::vector<std::string> &tokens, CondCmd &ret_cmd)
{
    std::string cond;

    if(tokens.empty() || tokens.size() > 1)
        return false;

    cond = tokens[0];
    toLower(cond);

    ret_cmd = enumerateCmd(cond);

    return ret_cmd == CondCmd_Endif || ret_cmd == CondCmd_IW_Endif || ret_cmd == CondCmd_IN_Endif;
}

enum CondLineTail
{
    COND_LineTail_None = 0,
    COND_LineTail_NewLine,
    COND_LineTail_Space
};

static CondLineTail replace_newline(CondCmd cmd)
{
    switch(cmd)
    {
    case CondCmd_IW_If:
    case CondCmd_IW_Endif:
        return COND_LineTail_Space;

    case CondCmd_IN_If:
    case CondCmd_IN_Endif:
        return COND_LineTail_None;

    default:
        return COND_LineTail_NewLine;
    }
}

void msgMacroProcess(const std::string &in, std::string &ret, int macro_player, int macro_state)
{
    struct State
    {
        bool open = false;
        bool cond_true = false;
        bool skip_to_endif = false;
        CondCmd init_cmd = CondCmd_Unknown;
    };

    if(in.empty())
        return;

    State st;
    std::vector<std::string> tokens;
    bool hasMacro = false;
    CondCmd e_cmd = CondCmd_Unknown;
    CondFunc e_func = CondFunc_Unknown;
    std::vector<int> values;
    CondLineTail tail = COND_LineTail_NewLine;
    std::string::size_type beg = 0;
    std::string::size_type end = 0;
    std::string t;

    t.reserve(100);

    do
    {
        end = in.find('\n', beg);

        if(end == std::string::npos)
            end = in.size();

        t.assign(in.c_str() + beg, end - beg);
        beg = end + 1;

        values.clear();
        tokens.clear();
        e_cmd = CondCmd_Unknown;
        e_func = CondFunc_Unknown;

        hasMacro = msgMacroParseTokens(t, tokens);

        if(hasMacro && check_cond_if(tokens, st.open, e_cmd, e_func, values))
        {
            bool isValid = true;
            st.cond_true = false;

            if(st.open && st.skip_to_endif)
                continue;

            if(e_cmd == CondCmd_If || e_cmd == CondCmd_IW_If || e_cmd == CondCmd_IN_If)
                st.init_cmd = e_cmd;

            if(e_func == CondFunc_Player) // check whaever player
            {
                st.open = true;

                for(auto &i : values)
                {
                    if(i == macro_player)
                    {
                        st.cond_true = true;
                        st.skip_to_endif = true;
                        break;
                    }
                }
            }
            else if(e_func == CondFunc_State) // check whaever state
            {
                st.open = true;

                for(auto &i : values)
                {
                    if(i == macro_state)
                    {
                        st.cond_true = true;
                        st.skip_to_endif = true;
                        break;
                    }
                }
            }
            else // invalid line
            {
                isValid = false;
                if(!ret.empty())
                    ret.push_back('\n');
                ret.append(t);
            }

            if(!isValid)
                st.init_cmd = CondCmd_Unknown;
        }
        else if(hasMacro && st.open && check_cond_endif(tokens, e_cmd))
        {
            st.open = false;
            st.cond_true = false;
            st.skip_to_endif = false;
            st.init_cmd = e_cmd;
        }
        else if(hasMacro && st.open && check_cond_else(tokens, e_cmd))
        {
            st.open = true;
            st.cond_true = !st.skip_to_endif;
            if(st.open && st.skip_to_endif)
                continue;

            st.skip_to_endif = false;
        }
        else if(!st.open || st.cond_true) // ordinary line
        {
            if(!ret.empty())
            {
                tail = replace_newline(st.init_cmd);
                st.init_cmd = CondCmd_Unknown;

                switch(tail)
                {
                case COND_LineTail_NewLine:
                    ret.push_back('\n');
                    break;
                case COND_LineTail_Space:
                    ret.push_back(' ');
                    break;
                case COND_LineTail_None:
                    break;
                }
            }

            tail = COND_LineTail_NewLine;
            ret.append(t);
        }
    } while(end < in.size() - 1);
}
