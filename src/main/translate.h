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

#pragma once
#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <map>
#include <string>

#include "editor/editor_custom.h"

const std::string& LanguageFormatNumber(int number, const std::string& singular, const std::string& dual, const std::string& plural);

class XTechTranslate
{
    typedef std::map<std::string, std::string*> TrList;
    //! Map of the engine specific translation key and the actual string container, used to simplify the maintenance
    TrList m_engineMap;

    //! Map of the assets specific translation key and the actual string container, used to simplify the maintenance
    TrList m_assetsMap;

public:
    XTechTranslate();

    /*!
     * \brief Load default built-in translation
     */
    void reset();

    /*!
     * \brief Print the content of default translation file to the terminal
     */
    void exportTemplate();

    /*!
     * \brief Fetch all translation files and update their content (add any missing entries)
     */
    void updateLanguages(const std::string& outPath, bool noBlank);

    /*!
     * \brief Switch the language of the engine
     * \return True if language was been successfully loaded
     */
    bool translate();

#ifdef THEXTECH_ENABLE_EDITOR
    friend void EditorCustom::Load(XTechTranslate* translate);
    friend struct EditorCustom::ItemList_t;
#endif

private:
    bool translateFile(const std::string &file, TrList &list, const char* trTypeName);
};

// safe to call at any time; reloads all engine-level string translations (no level / episode translations)
void ReloadTranslations();

#endif // TRANSLATE_H
