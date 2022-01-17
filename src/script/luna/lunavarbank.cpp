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

#include "lunavarbank.h"
#include "lunadefs.h"
#include "globals.h"
#include <fmt_format_ne.h>
#include <Utils/files.h>

saveUserData::DataSection gLunaVarBank;
SavedVariableBank gSavedVarBank;

#define SPECIAL_SAVE_STR "__LunaVarBankSpecialCounter"

// INIT
void SavedVariableBank::Init()
{}

// TRY LOAD WORLD VARS
SavedVariableBank::SavedVariableBank()
{
    Init();
}

bool SavedVariableBank::TryLoadWorldVars()
{
    if(selSave > 3)
        return false;

    ClearBank();
    ImportBank();

    return true;
}

// INIT SAVE FILE
void SavedVariableBank::InitSaveFile()
{
    const char *lunaKey = "__Lunadll_Version";
    if(m_VarBank.find(lunaKey) == m_VarBank.end())
        m_VarBank.insert({lunaKey, LUNA_VERSION});
}

void SavedVariableBank::ImportBank()
{
    if(gLunaVarBank.data.empty())
        InitSaveFile();

    for(auto &d : gLunaVarBank.data)
        SetVar(d.key, std::atof(d.value.c_str()));

    lastStarsNumber = Maths::iRound(GetVar(SPECIAL_SAVE_STR));
}

// SET VAR
void SavedVariableBank::SetVar(const std::string &k, double v)
{
    m_VarBank[k] = v;
}

// GET VAR
double SavedVariableBank::GetVar(const std::string &key)
{
    if(!VarExists(key))
        return 0.0;
    return m_VarBank[key];
}

// VAR EXISTS
bool SavedVariableBank::VarExists(std::string k)
{
    if(m_VarBank.find(k) == m_VarBank.end())
        return false;
    return true;
}

// CLEAR BANK
void SavedVariableBank::ClearBank()
{
    lastStarsNumber = 0;
    m_VarBank.clear();
}

// WRITE BANK
void SavedVariableBank::WriteBank()
{
    if(selSave > 3)
        return;

    gLunaVarBank.name = "LunaDLL";
    gLunaVarBank.location = saveUserData::DATA_GLOBAL;
    gLunaVarBank.data.clear();

    for(auto &it : m_VarBank)
    {
        saveUserData::DataEntry de;
        de.key = it.first;
        de.value = fmt::format_ne("{0}", it.second);
        gLunaVarBank.data.push_back(std::move(de));
    }
}

// SaveIfNeeded
void SavedVariableBank::SaveIfNeeded()
{
    // Basically, force save if the player collected a star
    if(numStars > lastStarsNumber)
    {
        lastStarsNumber = numStars;
        SetVar(SPECIAL_SAVE_STR, numStars);
        WriteBank();
    }
}

//COPY BANK
void SavedVariableBank::CopyBank(std::map<std::string, double> *target_map)
{
    if(!target_map)
        return;

    for(auto &it : m_VarBank)
    {
        auto n = (*target_map).find(it.first);
        if(n == (*target_map).end())
            (*target_map).insert(it);
        else
            n->second = it.second;
    }
}
