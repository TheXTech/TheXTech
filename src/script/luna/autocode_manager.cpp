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

#include <Utils/files.h>
#include <Utils/dir_list_ci.h>
#include <AppPath/app_path.h>
#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_stdinc.h"
#include "core/msgbox.h"
#include "main/translate_episode.h"

#include "autocode_manager.h"
#include "globals.h"
#include "global_dirs.h"
#include "lunamisc.h"
#include "lunaspriteman.h"

#define PARSEDEBUG true

AutocodeManager gAutoMan;

// CTOR
AutocodeManager::AutocodeManager() noexcept
{
    Clear();
}

AutocodeManager::~AutocodeManager()
{
    Clear();
}

// CLEAR - Delete all autocodes and clear lists (and reset hearts to 2)
void AutocodeManager::Clear()
{
    // Clear global codes
    m_GlobalCodes.clear();
    m_GlobalEnabled = false;

    m_globcodeIdxRef.clear();
    m_globcodeIdxSection.clear();


    // Clear level local and index tables
    m_Autocodes.clear();
    m_InitAutocodes.clear();
    m_CustomCodes.clear();

    m_autocodeIdxRef.clear();
    m_autocodeIdxSection.clear();

    m_Hearts = 2;

    m_Enabled = false;
}

bool AutocodeManager::LoadFiles()
{
    bool ret = false;
    std::string lunaLevel, lunaWorld,
                keyLevel, keyWorld;

    g_dirEpisode.setCurDir(FileNamePath);
    g_dirCustom.setCurDir(FileNamePath + FileName);

    Clear();

    // Load autocode
    lunaLevel = g_dirCustom.resolveFileCaseExistsAbs(AUTOCODE_FNAME);
    keyLevel = FileName + "/" + g_dirCustom.resolveFileCase(AUTOCODE_FNAME);
    if(!lunaLevel.empty())
        ret |= ReadFile(lunaLevel, keyLevel);

    // Try to load world codes
    lunaWorld = g_dirEpisode.resolveFileCaseExistsAbs(WORLDCODE_FNAME);
    keyWorld = g_dirEpisode.resolveFileCase(WORLDCODE_FNAME);
    if(!lunaWorld.empty())
        ret |= ReadWorld(lunaWorld, keyWorld);

    // Attempt to load global codes at the assets directory
    ret |= ReadGlobals(AppPathManager::assetsRoot() + GLOBALCODE_FNAME);

    // Do some stuff
    DoEvents(true); // do with init

    return ret;
}

// READ FILE - Read the autocode file in the level folder
bool AutocodeManager::ReadFile(const std::string &script_path, const std::string& tr_key)
{
    FILE *code_file = Files::utf8_fopen(script_path.c_str(), "rb");
    if(!code_file)
        return false;

    pLogDebug("Loading %s level local autocode script...", script_path.c_str());

    m_Enabled = true;
    Parse(code_file, false, tr_key);
    std::fclose(code_file);
    showErrors(script_path);

    return true;
}

// READ WORLD - Read the world autocode file in the world folder
bool AutocodeManager::ReadWorld(const std::string &script_path, const std::string& tr_key)
{
    FILE *code_file = Files::utf8_fopen(script_path.c_str(), "rb");
    if(!code_file)
        return false;

    pLogDebug("Loading %s episode wide autocode script...", script_path.c_str());

    m_Enabled = true;
    Parse(code_file, false, tr_key);
    std::fclose(code_file);
    showErrors(script_path);

    return true;
}

// READ GLOBALS - Read the global code file
bool AutocodeManager::ReadGlobals(const std::string &script_path)
{
    FILE *code_file = Files::utf8_fopen(script_path.c_str(), "rb");
    if(!code_file)
        return false;

    pLogDebug("Loading %s global autocode script...", script_path.c_str());

    m_Enabled = true;
    Parse(code_file, true);
    std::fclose(code_file);
    m_GlobalEnabled = true;
    showErrors(script_path);

    return true;
}

// PARSE    - Parse the autocode file and populate manager with the contained code/settings
//            Doesn't delete codes already in the lists
void AutocodeManager::Parse(FILE *code_file, bool add_to_globals, const std::string& tr_key)
{
    char wbuf[2000];
    char wmidbuf[2000];
    size_t wbuflen = 0;
    char combuf[150];
    int cur_section = 0;
    AutocodeType ac_type = AT_Invalid;
    double target = 0;
    double param1 = 0;
    double param2 = 0;
    double param3 = 0;
    double length = 0;
    int btarget = 0;
    int bparam1 = 0;
    int bparam2 = 0;
    int bparam3 = 0;
    int blength = 0;

    char wstrbuf[1000];
    char wrefbuf[128];
    int lineNum = 0;

    SDL_memset(wbuf, 0, 2000);
    SDL_memset(combuf, 0, 150);
    SDL_memset(wstrbuf, 0, 1000);
    SDL_memset(wrefbuf, 0, 128);

    m_errors.clear();

    TranslateEpisode tr;
    tr.loadLunaScript(tr_key);

    std::fseek(code_file, 0, SEEK_SET);

    // Check and skip a BOM marker
    const char *charset;
    if(Files::skipBom(code_file, &charset) != Files::CHARSET_UTF8)
    {
        addError(lineNum, charset, "File uses an unsupported charset. Please save it as UTF-8.");
        return;
    }

    //char* dbg = "ParseDbgEOF";
    while(!std::feof(code_file))
    {
        // Get a line and reset buffers
        SDL_memset(wbuf, 0, sizeof(wbuf));
        SDL_memset(wmidbuf, 0, sizeof(wmidbuf));
        SDL_memset(wstrbuf, 0, sizeof(wstrbuf));
        SDL_memset(wrefbuf, 0, sizeof(wrefbuf));
        SDL_memset(combuf, 0, sizeof(combuf));
        target = 0;
        param1 = 0;
        param2 = 0;
        param3 = 0;
        length = 0;
        btarget = 0;
        bparam1 = 0;
        bparam2 = 0;
        bparam3 = 0;
        blength = 0;

        if(!std::fgets(wbuf, 2000, code_file))
            break; // End of file has reached
        lineNum++;

        // Is it a comment?
        char *commentLine = SDL_strstr(wbuf, "//");
        if(commentLine != nullptr)
            commentLine[0] = '\0'; // Cut the comment line at here

        // does this line contains anything useful?
        wbuflen = SDL_strlen(wbuf);
        while(wbuflen > 0)
        {
            auto c = wbuf[wbuflen - 1];
            if(c != '\n' && c != '\r' && c != '\t' && c != ' ')
                break;
            wbuf[wbuflen - 1] = '\0';
            wbuflen--;
        }

        // Is it an empty line?
        if(wbuflen <= 0)
            continue;

        // Is it a new section header?
        if(wbuf[0] == '#')
        {
            // Is it the level load header?
            if(wbuf[1] == '-')
                cur_section = -1;
            else if(SDL_strncasecmp(wbuf + 1, "end", sizeof(wbuf) - 1) == 0)
                continue; // "END" keyword, just do nothing
            else // Else, parse the section number
            {
                try
                {
                    cur_section = std::stoi(wbuf + 1);
                }
                catch (const std::exception &e)
                {
                    addError(lineNum, wbuf, fmt::format_ne("Bad section format ({0})", e.what()));
                    // Keep section number unchanged
                }
            }
        }
        else   // Else, parse as a command
        {
            // Is there a variable reference marker?
            if(wbuf[0] == '$')
            {
                int i = 1;
                while(wbuf[i] != ',' && wbuf[i] != '\x0D' && wbuf[i] != '\x0A' && i < 126)
                    ++i;

                wbuf[i] = '\x00'; // Turn the comma into a null terminator
                SDL_strlcpy(wrefbuf, &wbuf[1], sizeof(wrefbuf)); // Copy new string into wrefbuf
                SDL_memcpy(wmidbuf, wbuf, sizeof(wmidbuf));
                SDL_strlcpy(wbuf, &wmidbuf[i + 1], sizeof(wbuf)); // The rest of the line minus the ref is the new wbuf
            }

            ac_type = Autocode::EnumerizeCommand(wbuf, lineNum);

            // Decimal pass
            int hits = XTECH_sscanf(wbuf, PARSE_FMT_STR, combuf, &target, &param1, &param2, &param3, &length, wstrbuf);

            // Integer pass
            int bhits = XTECH_sscanf(wbuf, PARSE_FMT_STR_2, combuf, &btarget, &bparam1, &bparam2, &bparam3, &blength, wstrbuf);

            // Check for formatting failure
            if(hits < 3 && bhits < 3)
            {
                addError(lineNum, wbuf, "Bad line format");
                continue;
            }

            // Check for hexadecimal inputs
            //if(true) // Always true
            {
                if(target == 0 && btarget != 0)
                    target = btarget;
                if(param1 == 0 && bparam1 != 0)
                    param1 = bparam1;
                if(param2 == 0 && bparam2 != 0)
                    param2 = bparam2;
                if(param3 == 0 && bparam3 != 0)
                    param3 = bparam3;
                if(length == 0 && blength != 0)
                    length = blength;
            }

            // Register new autocode

            std::string ac_str = std::string(wstrbuf); // Get the string out of strbuf

            switch(ac_type) // Translate string if possible
            {
            case AT_ShowText:
            case AT_ShowVar:
                tr.trScriptLine(ac_str, lineNum);
                break;
            default:
                break;
            }

            ac_str.erase(ac_str.find_last_not_of(" \n\r\t") + 1);

            std::string ref_str = std::string(wrefbuf); // Get var reference string if any

            Autocode newcode(ac_type, target, param1, param2, param3, AllocS(ac_str), length, cur_section, AllocS(ref_str));
            if(!add_to_globals)
            {
                if(newcode.m_Type < 10000 || newcode.MyRef != STRINGINDEX_NONE)
                {
                    m_Autocodes.emplace_back(std::move(newcode));
                    addToIndex(&m_Autocodes.back());
                }
                else   // Sprite components (type 10000+) with no reference go into callable component list
                    gSpriteMan.m_ComponentList.push_back(Autocode::GenerateComponent(newcode));
            }
            else
            {
                if(newcode.m_Type < 10000)
                {
                    m_GlobalCodes.emplace_back(std::move(newcode));
                    addToIndexGlob(&m_GlobalCodes.back());
                }
            }
        }
    }//while
}

std::string AutocodeManager::resolveWorldFileCase(const std::string &in_name)
{
    return g_dirEpisode.resolveFileCaseExistsAbs(in_name);
}

std::string AutocodeManager::resolveCustomFileCase(const std::string &in_name)
{
    return g_dirCustom.resolveFileCaseExistsAbs(in_name);
}

// DO EVENTS
void AutocodeManager::DoEvents(bool init)
{
    //char* dbg = "DO EVENTS DBG";
    if(m_Enabled)
    {
        // Add any outstanding custom events
        while(!m_CustomCodes.empty())
        {
            m_Autocodes.push_back(m_CustomCodes.back());
            m_CustomCodes.pop_back();
            addToIndex(&m_Autocodes.back());
        }

        // Do each code
        for(auto &m_Autocode : m_Autocodes)
            m_Autocode.Do(init);
    }

    if(m_GlobalEnabled)
    {
        // Do each global code
        for(auto &m_GlobalCode : m_GlobalCodes)
            m_GlobalCode.Do(init);
    }
}

// GET EVENT BY REF
Autocode *AutocodeManager::GetEventByRef(const std::string &ref_name)
{
    if(ref_name.length() > 0)
    {
        auto ref = m_autocodeIdxRef.find(ref_name);
        if(ref == m_autocodeIdxRef.end() || ref->second.empty())
            return nullptr;

        return ref->second.front();
    }
    return nullptr;
}

// DELETE EVENT -- Expires any command that matches the given name
void AutocodeManager::DeleteEvent(const std::string &ref_name)
{
    if(ref_name.length() > 0)
    {
        auto ref = m_autocodeIdxRef.find(ref_name);
        if(ref != m_autocodeIdxRef.end())
        {
            for(auto *code : ref->second)
                code->expire();
        }
    }
}

// CLEAN EXPIRED - Don't call this while iterating over codes
void AutocodeManager::ClearExpired()
{
    if(!m_hasExpired)
        return; // Nothing to do

// #define DEBUG_CLEAN_EXPIRED

#ifdef DEBUG_CLEAN_EXPIRED
    int cleanedAutos = 0, cleanedGlobs = 0;
#endif

    //char* dbg = "CLEAN EXPIRED DBG";
    auto iter = m_Autocodes.begin();
    auto end  = m_Autocodes.end();

    while(iter != m_Autocodes.end())
    {
        //Autocode* ac = *iter;
        if((*iter).Expired || (*iter).m_Type == AT_Invalid)
        {
            removeFromIndex(&*iter);
//            delete(*iter);
            iter = m_Autocodes.erase(iter);
#ifdef DEBUG_CLEAN_EXPIRED
            cleanedAutos++;
#endif
        }
        else
            ++iter;
    }

    iter = m_GlobalCodes.begin();
    end  = m_GlobalCodes.end();

    while(iter != m_GlobalCodes.end())
    {
        //Autocode* ac = *iter;
        if((*iter).Expired || (*iter).m_Type == AT_Invalid)
        {
//            delete(*iter);
            removeFromIndexGlob(&*iter);
            iter = m_GlobalCodes.erase(iter);
#ifdef DEBUG_CLEAN_EXPIRED
            cleanedGlobs++;
#endif
        }
        else
            ++iter;
    }

#ifdef DEBUG_CLEAN_EXPIRED
    if(cleanedAutos > 0)
        D_pLogDebug("Autocode: Cleaned %d expired autocodes", cleanedAutos);

    if(cleanedGlobs > 0)
        D_pLogDebug("Autocode: Cleaned %d expired global autocodes", cleanedGlobs);
#endif

    m_hasExpired = false;
}

// ACTIVATE CUSTOM EVENTS
void AutocodeManager::ActivateCustomEvents(int new_section, int eventcode)
{
    //char* dbg = "ACTIVATE CUSTOM DBG";
    if(m_Enabled)
    {
        auto secA = m_autocodeIdxSection.find(eventcode);
        if(secA != m_autocodeIdxSection.end())
        {
            for(auto *code : secA->second)
            {
                // Activate copies of events with 'eventcode' and move them to 'new_section'
                if(!code->Activated && !code->Expired)
                {
                    Autocode newcode = *code;
                    newcode.Activated = true;
                    newcode.ActiveSection = (new_section < 1000 ? (new_section - 1) : new_section);
                    newcode.Length = code->m_OriginalTime;
                    m_CustomCodes.push_front(std::move(newcode));
                }
            }
        }

        auto secG = m_globcodeIdxSection.find(eventcode);
        if(secG != m_globcodeIdxSection.end())
        {
            for(auto *code : secG->second)
            {
                // Activate copies of events with 'eventcode' and move them to 'new_section'
                if(!code->Activated && !code->Expired)
                {
                    Autocode newcode = *code;
                    newcode.Activated = true;
                    newcode.ActiveSection = (new_section < 1000 ? (new_section - 1) : new_section);
                    newcode.Length = code->m_OriginalTime;
                    m_CustomCodes.push_front(std::move(newcode));
                }
            }
        }
    }
}

// FORCE EXPIRE -- Expire all codes in section
void AutocodeManager::ForceExpire(int section)
{
    //char* dbg = "FORCE EXPIRE DBG";
    if(m_Enabled)
    {
        auto sec = m_autocodeIdxSection.find(section);
        if(sec != m_autocodeIdxSection.end())
        {
            for(auto *code : sec->second)
                code->expire();
        }

        auto secG = m_globcodeIdxSection.find(section);
        if(secG != m_globcodeIdxSection.end())
        {
            for(auto *code : secG->second)
                code->expire();
        }
    }
}

// FIND MATCHING -- Return a reference to the first autocode that matches, or 0
Autocode *AutocodeManager::FindMatching(int section, const std::string &soughtstr)
{
    //char* dbg = "FIND MATCHING DBG";
    // Find at m_Autocodes
    auto s = m_autocodeIdxSection.find(section);
    if(s == m_autocodeIdxSection.end())
        return nullptr;

    for(auto *code : s->second)
    {
        if(GetS(code->MyString) == soughtstr)
            return code;
    }

    return nullptr;
}

// VAR OPERATION -- Do something to a variable in the user variable bank
bool AutocodeManager::VarOperation(const std::string &var_name, double value, OPTYPE operation_to_do)
{
    if(var_name.length() > 0)
    {
        // Create var if doesn't exist
        InitIfMissing(&gAutoMan.m_UserVars, var_name, 0);

        double var_val = m_UserVars[var_name];

        // Do the operation
        OPTYPE oper = operation_to_do;
        switch(oper)
        {
        case OP_Assign:
            m_UserVars[var_name] = value;
            return true;
        case OP_Add:
            m_UserVars[var_name] = var_val + value;
            return true;
        case OP_Sub:
            m_UserVars[var_name] = var_val - value;
            return true;
        case OP_Mult:
            m_UserVars[var_name] = var_val * value;
            return true;
        case OP_Div:
            if(value == 0)
                return false;
            m_UserVars[var_name] = var_val / value;
            return true;
        case OP_XOR:
            m_UserVars[var_name] = (int)var_val ^ (int)value;
            return true;
        default:
            return true;
        }
    }
    return false;
}

void AutocodeManager::addToIndex(Autocode *code)
{
    m_autocodeIdxSection[code->ActiveSection].push_back(code);
    if(!GetS(code->MyRef).empty())
        m_autocodeIdxRef[GetS(code->MyRef)].push_back(code);
}

void AutocodeManager::removeFromIndex(Autocode *code)
{
    auto &sec = m_autocodeIdxSection[code->ActiveSection];
    for(auto it = sec.begin(); it != sec.end(); )
    {
        if(*it == code)
            it = sec.erase(it);
        else
            ++it;
    }

    if(!GetS(code->MyRef).empty())
    {
        auto &ref = m_autocodeIdxRef[GetS(code->MyRef)];
        for(auto it = ref.begin(); it != ref.end(); )
        {
            if(*it == code)
                it = ref.erase(it);
            else
                ++it;
        }
    }
}

void AutocodeManager::addToIndexGlob(Autocode *code)
{
    m_globcodeIdxSection[code->ActiveSection].push_back(code);
    if(!GetS(code->MyRef).empty())
        m_globcodeIdxRef[GetS(code->MyRef)].push_back(code);
}

void AutocodeManager::removeFromIndexGlob(Autocode *code)
{
    auto &sec = m_globcodeIdxSection[code->ActiveSection];
    for(auto it = sec.begin(); it != sec.end(); )
    {
        if(*it == code)
            it = sec.erase(it);
        else
            ++it;
    }

    if(!GetS(code->MyRef).empty())
    {
        auto &ref = m_globcodeIdxRef[GetS(code->MyRef)];
        for(auto it = ref.begin(); it != ref.end(); )
        {
            if(*it == code)
                it = ref.erase(it);
            else
                ++it;
        }
    }
}

void AutocodeManager::addError(int lineNumber, const std::string &line, const std::string &msg)
{
    ParseError err;
    err.lineNumber = lineNumber;
    err.line = line;
    err.message = msg;

    // Trim the tail of the line
    while(!err.line.empty())
    {
        auto c = err.line.back();
        if(c != '\n' && c != '\r' && c != '\t' && c != ' ')
            break;
        err.line.pop_back();
    }

    pLogWarning("Autocode: Script parse error: [%s] at %d: %s",
                err.message.c_str(), err.lineNumber, err.line.c_str());

    m_errors.push_back(std::move(err));
}

void AutocodeManager::showErrors(const std::string &file)
{
    if(m_errors.empty())
        return; // No errors

    std::string out = fmt::format_ne("While parsing the {0} file, next errors has ocurred:\n\n", file);

    int lines = 0;
    for(auto &e : m_errors)
    {
        out += fmt::format_ne("{0}, {1}:{2}\n", e.message, e.lineNumber, e.line);
        lines++;
        if(lines > 25)
        {
            out += "... ";
            break;
        }
    }

    XMsgBox::simpleMsgBox(XMsgBox::MESSAGEBOX_ERROR, "Autocode script parse errors", out);
}

// VAR EXISTS
bool AutocodeManager::VarExists(const std::string &var_name)
{
    return (m_UserVars.find(var_name) != m_UserVars.end());
}

// GET VAR
double AutocodeManager::GetVar(const std::string &var_name)
{
    if(!VarExists(var_name))
        return 0;

    return m_UserVars[var_name];
}
