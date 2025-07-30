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
#include <climits>
#include <array>
#include "msg_macros.h"

#ifdef MOONDUST_MSG_MACROS_ERROR_HANDLING
#   define MSG_MACRO_ERROR_ARG , MsgMacroErrors *error
#   define D_ERROR(arg) *error = arg
#   define D_ELSE_SEEN(arg) else_seen = arg
#   define D_CHECK_ELSE_SEEN else_seen
#else
#   define MSG_MACRO_ERROR_ARG
#   define D_ERROR(arg) (void)1
#   define D_ELSE_SEEN(arg) (void)1
#   define D_CHECK_ELSE_SEEN false
#endif


static void skip_spaces(const char*& buffer)
{
    while(*buffer == ' ')
        buffer++;
}

struct Keyword
{
    const char* str;
    const int length;
};

static int enumerateKeyword(const char*& buffer, const Keyword* keywords, int num_keywords)
{
    skip_spaces(buffer);

    for(int keyword_i = 0; keyword_i < num_keywords; keyword_i++)
    {
        const Keyword& keyword = keywords[keyword_i];

        // check whether keyword is a prefix of buffer
        if(strncmp(buffer, keyword.str, keyword.length) != 0)
            continue;

        // check whether buffer has a token separator following keyword (more validation can be done later)
        const char end = buffer[keyword.length];
        if(!(end == '\n' || end == ' ' || end == '\0' || end == '(' || end == ',' || end == ')'))
            continue;

        buffer += keyword.length;
        skip_spaces(buffer);

        return keyword_i;
    }

    return -1;
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
    // dummy limit item
    CondCmd_MAX,
};

const std::array<Keyword, CondCmd_MAX> commands = {Keyword{"if", 2}, Keyword{"elif", 4}, Keyword{"else", 4}, Keyword{"endif", 5}, Keyword{"if_iw", 5}, Keyword{"if_in", 5}, Keyword{"endif_iw", 8}, Keyword{"endif_in", 8}};

enum CondFunc
{
    CondFunc_Unknown = -1,
    CondFunc_Player = 0,
    CondFunc_State,
    CondFunc_MAX,
};

const std::array<Keyword, CondFunc_MAX> funcs = {Keyword{"player", 6}, Keyword{"state", 5}};

static CondCmd enumerateCmd(const char*& buffer)
{
    return (CondCmd)enumerateKeyword(buffer, commands.data(), commands.size());
}

static CondFunc enumerateFunc(const char*& buffer)
{
    return (CondFunc)enumerateKeyword(buffer, funcs.data(), funcs.size());
}

static int parseValue(const char*& buffer)
{
    skip_spaces(buffer);
    if(!std::isdigit(*buffer))
        return INT_MIN;

    int ret = std::atoi(buffer);

    while(std::isdigit(*buffer))
        buffer++;

    return ret;
}

void msgMacroProcess(const std::string &in, std::string &ret, int macro_player, int macro_state MSG_MACRO_ERROR_ARG)
{
    enum class Status
    {
        outside = 0,   // outside of if/else clause, lines go to output, wait for #if
        inside,        // inside an active clause, lines go to output, wait for #elif / #else / #endif
        skip_to_else,  // no clause has been triggered yet, look for next elif/else
        skip_to_endif, // a clause has been triggered, look for else
    } status = Status::outside;

    bool line_start = true;

#ifdef MOONDUST_MSG_MACROS_ERROR_HANDLING
    bool else_seen = false;
#endif

    for(const char* in_ptr = in.c_str(); *in_ptr != '\0';)
    {
        char cur = *in_ptr;
        in_ptr++;

        // check if this is a macro
        bool is_macro = (line_start && cur == '#');

        if(!is_macro)
        {
            if(status == Status::outside || status == Status::inside)
                ret.push_back(cur);

            line_start = (cur == '\n');
            continue;
        }

        // exit a current block
        if(status == Status::inside)
            status = Status::skip_to_endif;

        // advances in_ptr
        CondCmd cmd = enumerateCmd(in_ptr);

        if(cmd == CondCmd_Unknown)
        {
            D_ERROR(MSG_MACRO_ERROR_UNKNOWN_CMD);
            return;
        }
        else if(cmd == CondCmd_Endif || cmd == CondCmd_IW_Endif || cmd == CondCmd_IN_Endif)
        {
            if(status == Status::outside)
            {
                D_ERROR(MSG_MACRO_ERROR_ILLEGAL_CMD);
                return;
            }

            // if anything has been written from the current condition
            if(ret.size() && ret.back() == '\n' && status != Status::skip_to_else)
            {
                if(cmd == CondCmd_IN_Endif)
                    ret.pop_back();
                else if(cmd == CondCmd_IW_Endif)
                    ret.back() = ' ';
            }

            D_ELSE_SEEN(false);
            status = Status::outside;
        }
        else if(cmd == CondCmd_Else)
        {
            if(status == Status::outside || D_CHECK_ELSE_SEEN)
            {
                D_ERROR(MSG_MACRO_ERROR_ILLEGAL_CMD);
                return;
            }

            D_ELSE_SEEN(true);

            // enter the else clause if we haven't already matched
            if(status != Status::skip_to_endif)
                status = Status::inside;
        }
        // condition macros
        else
        {
            if(cmd == CondCmd_If || cmd == CondCmd_IW_If || cmd == CondCmd_IN_If)
            {
                if(status != Status::outside)
                {
                    D_ERROR(MSG_MACRO_ERROR_ILLEGAL_CMD);
                    return;
                }

                if(ret.size() && ret.back() == '\n')
                {
                    if(cmd == CondCmd_IN_If)
                        ret.pop_back();
                    else if(cmd == CondCmd_IW_If)
                        ret.back() = ' ';
                }
            }
            else if(status == Status::outside)
            {
                D_ERROR(MSG_MACRO_ERROR_ILLEGAL_CMD);
                return;
            }
            else if(D_CHECK_ELSE_SEEN)
            {
                D_ERROR(MSG_MACRO_ERROR_ILLEGAL_CMD);
                return;
            }

            // -- CONDITION CHECK CODE --
            // we have a condition to check, and decide whether to enter Status::inside or Status::skip_to_else
            CondFunc func = enumerateFunc(in_ptr);

            int true_value = 0;
            if(func == CondFunc_Unknown)
            {
                D_ERROR((*in_ptr == '\n') ? MSG_MACRO_ERROR_BAD_CMD_SYNTAX : MSG_MACRO_ERROR_UNKNOWN_FUNC);
                return;
            }
            else if(func == CondFunc_Player)
                true_value = macro_player;
            else if(func == CondFunc_State)
                true_value = macro_state;

            if(*(in_ptr++) != '(')
            {
                D_ERROR(MSG_MACRO_ERROR_BAD_FUNC_SYNTAX);
                return;
            }

            // look for value that matches true_value
            bool matched = false;
            while(true)
            {
                int value = parseValue(in_ptr);
                if(value == INT_MIN)
                {
                    D_ERROR((*in_ptr == '\n' || *in_ptr == '(' || *in_ptr == ',' || *in_ptr == ')') ? MSG_MACRO_ERROR_BAD_FUNC_SYNTAX : MSG_MACRO_ERROR_BAD_FUNC_ARGS);
                    return;
                }

#ifdef MOONDUST_MSG_MACROS_ERROR_HANDLING
                bool has_trailing_space = (*in_ptr == ' ');
#endif
                skip_spaces(in_ptr);

                if(value == true_value)
                    matched = true;

                char delim = *(in_ptr++);
                switch(delim)
                {
                case ')':
                    // double break below
                    break;
                case ',':
                    continue;
                default:
                    D_ERROR((delim == '\n' || delim == '(' || delim == '\0' || has_trailing_space) ? MSG_MACRO_ERROR_BAD_FUNC_SYNTAX : MSG_MACRO_ERROR_BAD_FUNC_ARGS);

                    return;
                }

                // case ')' from above
                break;
            }

            skip_spaces(in_ptr);

            if(status != Status::skip_to_endif)
            {
                if(matched)
                    status = Status::inside;
                else
                    status = Status::skip_to_else;
            }
        }

        // done with macro, time to proceed
        if(*in_ptr == '\0')
            break;

        if(*in_ptr != '\n')
        {
            D_ERROR(MSG_MACRO_ERROR_EXTRA_SYMBOLS_AT_END);
            return;
        }

        line_start = true;
        in_ptr++;
    }

    // remove trailing whitespace for legacy purposes
    if(ret.size() && ret.back() == '\n')
        ret.pop_back();

    D_ERROR(MSG_MACRO_ERROR_OK);
}
