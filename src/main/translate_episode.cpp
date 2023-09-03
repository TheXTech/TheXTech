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

#include <json/json.hpp>
#include <Logger/logger.h>
#include <Utils/files.h>
#include <Utils/strings.h>
#include <fmt_format_ne.h>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "globals.h"
#ifndef LAYERS_H // Workaround for Clazy
#include "layers.h"
#endif

#include "fontman/font_manager.h"
#include "translate_episode.h"
#define XTECH_TRANSLATE_EPISODE
#include "translate/tr_title.h"
#include "translate/tr_level.h"
#include "translate/tr_world.h"
#include "translate/tr_script.h"


static std::string getTrFileLang(const std::string &lang, const std::string &dialect, const std::string &subDir, const std::string &episodePath = std::string())
{
    std::string langFile;

    const auto &p = episodePath.empty() ?
                        FileNamePath :
                        episodePath;

    // Trying to find the dialect-specific translation
    if(!dialect.empty())
    {
        if(!subDir.empty())
        {
            // Try to find the translation at the i18n at data sub-directory
            langFile = p + fmt::format_ne("{0}/i18n/translation_{1}-{2}.json", subDir, lang.c_str(), dialect.c_str());
            if(!Files::fileExists(langFile))
                langFile.clear();

            // Try to find the translation at the data sub-directory
            if(langFile.empty())
            {
                langFile = p + fmt::format_ne("{0}/translation_{1}-{2}.json", subDir, lang.c_str(), dialect.c_str());
                if(!Files::fileExists(langFile))
                    langFile.clear();
            }
        }

        // Now try at the i18n at episode root
        if(langFile.empty())
        {
            langFile = p + fmt::format_ne("i18n/translation_{0}-{1}.json", lang.c_str(), dialect.c_str());
            if(!Files::fileExists(langFile))
                langFile.clear();
        }

        // Now try at the episode root
        if(langFile.empty())
        {
            langFile = p + fmt::format_ne("translation_{0}-{1}.json", lang.c_str(), dialect.c_str());
            if(!Files::fileExists(langFile))
                langFile.clear();
        }
    }

    // Trying the general translation
    if(langFile.empty())
    {
        // Try to find the translation at the data sub-directory
        if(!subDir.empty())
        {
            // Try to find the translation at the i18n at data sub-directory
            langFile = p + fmt::format_ne("{0}/i18n/translation_{1}.json", subDir, lang.c_str());
            if(!Files::fileExists(langFile))
                langFile.clear();

            // Try to find the translation at the data sub-directory
            if(langFile.empty())
            {
                langFile = p + fmt::format_ne("{0}/translation_{1}.json", subDir, lang.c_str());
                if(!Files::fileExists(langFile))
                    langFile.clear();
            }
        }

        // Now try at the i18n at episode root
        if(langFile.empty())
        {
            langFile = p + fmt::format_ne("i18n/translation_{0}.json", lang.c_str());
            if(!Files::fileExists(langFile))
                langFile.clear();
        }

        // Now try at the episode root
        if(langFile.empty())
        {
            langFile = p + fmt::format_ne("translation_{0}.json", lang.c_str());
            if(!Files::fileExists(langFile))
                langFile.clear();
        }
    }

    return langFile;
}


static std::string getTrFile(const std::string &subDir, const std::string &episodePath = std::string())
{
    std::string langFile = getTrFileLang(CurrentLanguage, CurrentLangDialect, subDir, episodePath);

    if(langFile.empty()) // If no language detected or invalid language set, fallback to English
        langFile = getTrFileLang("en", "gb", subDir, episodePath);

    return langFile;
}


TranslateEpisode::TranslateEpisode()
{}

void TranslateEpisode::loadLevelTranslation(const std::string& key)
{
    if(!FontManager::isInitied() || FontManager::isLegacy())
    {
        pLogWarning("TranslateEpisode: Translations aren't supported without new font engine loaded (the 'fonts' directory is required)");
        return;
    }

    std::string langFile = getTrFile(FileName);

    if(langFile.empty())
        return; // No translation found

    TrLevelParser parser;
    parser.m_wantedKey = key;

    FILE *f_in = Files::utf8_fopen(langFile.c_str(), "rb");
    if(!f_in)
        return;

    if(nlohmann::json::sax_parse(f_in, &parser))
        pLogDebug("JSON SaX returned TRUE");
    else
        pLogDebug("JSON SaX returned FALSE");
}

void TranslateEpisode::loadWorldTranslation(const std::string& key)
{
    if(!FontManager::isInitied() || FontManager::isLegacy())
    {
        pLogWarning("TranslateEpisode: Translations aren't supported without new font engine loaded (the 'fonts' directory is required)");
        return;
    }

    std::string langFile = getTrFile(FileNameWorld);

    if(langFile.empty())
        return; // No translation found

    TrWorldParser parser;
    parser.m_wantedKey = key;

    FILE *f_in = Files::utf8_fopen(langFile.c_str(), "rb");
    if(!f_in)
        return;

    if(nlohmann::json::sax_parse(f_in, &parser))
        pLogDebug("JSON SaX returned TRUE");
    else
        pLogDebug("JSON SaX returned FALSE");
}

void TranslateEpisode::loadLunaScript(const std::string& key)
{
    m_scriptLines.clear();
    m_scriptTrId.clear();

    if(!FontManager::isInitied() || FontManager::isLegacy())
    {
        pLogWarning("TranslateEpisode: Translations aren't supported without new font engine loaded (the 'fonts' directory is required)");
        return;
    }

    std::string langFile = getTrFile(FileName);

    if(langFile.empty())
        return; // No translation found

    TrScriptParser parser;
    parser.m_wantedKey = key;
    parser.m_outputLines = &m_scriptLines;
    parser.m_outputTrIdLines = &m_scriptTrId;

    FILE *f_in = Files::utf8_fopen(langFile.c_str(), "rb");
    if(!f_in)
        return;

    if(nlohmann::json::sax_parse(f_in, &parser))
        pLogDebug("JSON SaX returned TRUE");
    else
        pLogDebug("JSON SaX returned FALSE");
}

bool TranslateEpisode::tryTranslateTitle(const std::string& episodePath,
                                         const std::string& worldFile,
                                         std::string& output)
{
    if(!FontManager::isInitied() || FontManager::isLegacy())
    {
        pLogWarning("TranslateEpisode: Translations aren't supported without new font engine loaded (the 'fonts' directory is required)");
        return false;
    }

    std::string langFile = getTrFile(std::string(), episodePath);

    if(langFile.empty())
        return false; // No translation found

    TrTitleParser parser;
    parser.m_toWrite = &output;
    parser.m_wantedWorld = worldFile;

    FILE *f_in = Files::utf8_fopen(langFile.c_str(), "rb");
    if(!f_in)
        return false;

    if(nlohmann::json::sax_parse(f_in, &parser))
        pLogDebug("JSON SaX returned TRUE");
    else
        pLogDebug("JSON SaX returned FALSE");

    return true;
}

void TranslateEpisode::trScriptLine(std::string& data, int line)
{
    if(!FontManager::isInitied() || FontManager::isLegacy())
    {
        pLogWarning("TranslateEpisode: Translations aren't supported without new font engine loaded (the 'fonts' directory is required)");
        return;
    }

    if(m_scriptLines.empty() && m_scriptTrId.empty())
        return;

    if(!m_scriptTrId.empty())
    {
        auto f = m_scriptTrId.find(data);
        if(f != m_scriptTrId.end())
        {
            data = f->second;
            return; // translated by TrId
        }
    }

    auto f = m_scriptLines.find(line);
    if(f != m_scriptLines.end())
        data = f->second; // translated by line number
}
