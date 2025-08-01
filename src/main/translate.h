/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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
#include "main/hints.h"

const std::string& LanguageFormatNumber(int number, const std::string& singular, const std::string& dual, const std::string& plural);

template<bool writable>
class _Config_t;

template<bool writable>
class BaseConfigOption_t;

template<bool writable>
class ConfigSection_t;

template<bool writable, class value_t>
class ConfigEnumOption_t;

class XTechTranslate
{
#ifdef THEXTECH_DISABLE_LANG_TOOLS
    typedef std::pair<std::string, std::string*> TrEntry;
    typedef std::vector<TrEntry> TrList;
#   define TR_MAP_TYPE 1
#   define TR_MAP_INSERT push_back
#else
    typedef std::map<std::string, std::string*> TrList;
#   define TR_MAP_TYPE 0
#   define TR_MAP_INSERT insert
#endif
    //! Map of the engine specific translation key and the actual string container, used to simplify the maintenance
    TrList m_engineMap;

    //! Map of the assets specific translation key and the actual string container, used to simplify the maintenance
    TrList m_assetsMap;

    /**
     * @brief Inserts a new translation line
     * @param list Translation list to insert
     * @param key Key of the translation line
     * @param value Pointer to the contained string that should be translated
     */
    static void insert(TrList &list, const char *key, std::string *value);

    /**
     * @brief Inserts a new translation line
     * @param list Translation list to insert
     * @param key Key of the translation line
     * @param value Pointer to the contained string that should be translated
     */
    static void insert(TrList &list, std::string &&key, std::string *value);

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

    friend void XHints::InitTranslations(XTechTranslate& translate);

    friend class _Config_t<false>;
    friend class BaseConfigOption_t<false>;
    friend class ConfigSection_t<false>;

    template<bool writable, class value_t>
    friend class ConfigEnumOption_t;

private:
    bool translateFile(const std::string &file, TrList &list, const char* trTypeName);
};

// safe to call at any time; reloads all engine-level string translations (no level / episode translations)
void ReloadTranslations();

#endif // TRANSLATE_H
