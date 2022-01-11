/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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
#ifndef AutoCodeManager_hhh
#define AutoCodeManager_hhh

#include <string>
#include <map>
#include <list>
#include <fstream>

#include "autocode.h"

#define AUTOCODE_FNAME      "lunadll.txt"
#define WORLDCODE_FNAME     "lunaworld.txt"
#define GLOBALCODE_FNAME    "lunaglobal.txt"
#define PARSE_FMT_STR       " %149[^,], %lf , %lf , %lf , %lf , %lf , %999[^\n]"
#define PARSE_FMT_STR_2     " %149[^,], %i , %i , %i , %i , %i , %999[^\n]"
//						 Cmd	Trg   P1    P2    P3    Len   String

struct AutocodeManager
{
    AutocodeManager();
    ~AutocodeManager();

    // File funcs
    void ReadFile(std::string dir_path); // Load level codes from dir_path
    void ReadWorld(std::string dir_path); // Load worldwide codes from dir_path
    void ReadGlobals(std::string dir_path); // Load global codes from dir_path
    void Parse(FILE *open_file, bool add_to_globals);

    // Management funcs
    void Clear(bool clear_global_codes);
    void ForceExpire(int section);
    void ClearExpired();
    void DeleteEvent(std::string event_reference_name);		// Look up event with given name and expire it
    void DoEvents(bool init);
    void ActivateCustomEvents(int new_section, int eventcode);
    Autocode* GetEventByRef(std::string event_reference_name);	// Return ptr to event with the given ref, or NULL if it fails
    Autocode* FindMatching(int section, std::string string);

    // Variable bank funcs
    double GetVar(std::string var_name);		// returns 0 if var doesn't exist in bank
    bool VarExists(std::string var_name);
    bool VarOperation(std::string var_name, double value, OPTYPE operation_to_do);

    // Members
    bool					m_Enabled;			// Whether or not individual level scripts enabled
    bool					m_GlobalEnabled;	// Whether or not global game scripts enabled
    std::list<Autocode*>	m_Autocodes;
    std::list<Autocode*>	m_InitAutocodes;
    std::list<Autocode*>	m_CustomCodes;
    std::list<Autocode*>	m_GlobalCodes;

    std::map<std::string, double> m_UserVars;

    // Hearts manager stuff
    int m_Hearts;
};

extern AutocodeManager gAutoMan;

#endif // AutoCodeManager_hhh
