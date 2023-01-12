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
#ifndef LUNAVARBANK_H
#define LUNAVARBANK_H

#include <string>
#include <map>
#include <PGE_File_Formats/save_filedata.h>

extern saveUserData::DataSection gLunaVarBank;

// -- Custom user variables saving/loading manager --
// How it works: (On level load) Reads global "LunaDLL" section from the game save and reads all vars into object's local variable bank
// Whole variable bank will be written back to the gamesave when calling WriteBank
// Object is basically a wrapper around a map with some file management functions
class SavedVariableBank
{
    int lastStarsNumber = 0;
public:
    SavedVariableBank();

    /*!
     * \brief Try to read in the saved variables for this world & current save slot
     * \return True on success, false on error
     */
    bool TryLoadWorldVars();

    /*!
     * \brief Sets or adds a key/value pair to the bank
     * \param key Key
     * \param val Value
     */
    void SetVar(const std::string &key, double val);

    /*!
     * \brief Returns whether or not this var exists in the bank
     * \param key Key
     * \return true if exists
     */
    bool VarExists(const std::string &key);

    /*!
     * \brief Get value of a key, or 0 if key not found
     * \param key Key
     * \return Value
     */
    double GetVar(const std::string &key);

    /*!
     * \brief Copy all k,v pairs of variable bank to another map
     * \param target Destination map of copy
     */
    void CopyBank(std::map<std::string, double> *target);

    void ClearBank();

    /*!
     * \brief Save vars by writing the current bank back to world folder
     */
    void WriteBank();

    /*!
     * \brief Saves if it detects the player has collected a new star
     */
    void SaveIfNeeded();

    /// Members ///
    std::map<std::string, double> m_VarBank;

private:
    // Init the object
    void Init();
    // Init a new save file
    void InitSaveFile();
    // Read all vars from open file into bank
    void ImportBank();
};

extern SavedVariableBank gSavedVarBank;

#endif // LUNAVARBANK_H
