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

#include <algorithm>
#include <Utils/strings.h>
#include <Utils/files.h>
#include <util.h>
#include <DirManager/dirman.h>
#include <AppPath/app_path.h>
#include <fmt_format_ne.h>

#ifndef THEXTECH_DISABLE_SDL_LOCALE
#   include <SDL2/SDL_version.h>
#   if SDL_COMPILEDVERSION >= SDL_VERSIONNUM(2, 0, 14)
#       include <SDL2/SDL_locale.h>
#   else
#       define THEXTECH_DISABLE_SDL_LOCALE
#   endif
#endif

#include "sdl_proxy/sdl_rwops.h"

#include "config.h"
#include "globals.h"
#include "core/language.h"
#include "core/language/language_private.h"


static std::vector<std::string> s_languages;

static std::string langEngineFile;
static std::string langAssetsFile;


static bool detectSetup()
{
    if(CurrentLanguage.empty())
        return false; // Language code is required!

    std::transform(CurrentLanguage.begin(),
                   CurrentLanguage.end(),
                   CurrentLanguage.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::transform(CurrentLangDialect.begin(),
                   CurrentLangDialect.end(),
                   CurrentLangDialect.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    langEngineFile.clear();
    langAssetsFile.clear();

    if(!CurrentLangDialect.empty())
    {
        langEngineFile = AppPathManager::languagesDir() + fmt::format_ne("thextech_{0}-{1}.json", CurrentLanguage.c_str(), CurrentLangDialect.c_str());
        langAssetsFile = AppPathManager::languagesDir() + fmt::format_ne("assets_{0}-{1}.json", CurrentLanguage.c_str(), CurrentLangDialect.c_str());
        if(!Files::fileExists(langEngineFile))
            langEngineFile.clear();
        if(!Files::fileExists(langAssetsFile))
            langAssetsFile.clear();
    }

    if(langEngineFile.empty())
        langEngineFile = AppPathManager::languagesDir() + fmt::format_ne("thextech_{0}.json", CurrentLanguage.c_str());
    if(langAssetsFile.empty())
        langAssetsFile = AppPathManager::languagesDir() + fmt::format_ne("assets_{0}.json", CurrentLanguage.c_str());

    if(!Files::fileExists(langEngineFile))
        langEngineFile.clear();
    if(!Files::fileExists(langAssetsFile))
        langAssetsFile.clear();

    pLogDebug("Trying localization %s-%s -> found files [%s] and [%s]", CurrentLanguage.c_str(), CurrentLangDialect.c_str(), langEngineFile.c_str(), langAssetsFile.c_str());

    return !langEngineFile.empty() || !langAssetsFile.empty();
}


void XLanguage::findLanguages()
{
    constexpr size_t prefix_length = 9; // std::string("thextech_").size();

    // clear and free s_languages
    util::clear_mem(s_languages);

    std::vector<std::string> list;
    DirMan langs(AppPathManager::languagesDir());

    if(!langs.exists())
    {
        pLogDebug("Can't open the languages directory: %s", langs.absolutePath().c_str());
        return;
    }

    if(!langs.getListOfFiles(list, {".json"}))
    {
        pLogDebug("Can't show the content of the languages directory: %s", langs.absolutePath().c_str());
        return;
    }

    if(list.empty())
    {
        pLogDebug("Languages directory does not have JSON files: %s", langs.absolutePath().c_str());
        return;
    }

    // tag non-lowercase filenames and language codes including underscores for exclusion during sorting
    for(std::string& fn : list)
    {
        for(auto& c : fn)
        {
            if(c != std::tolower(c))
            {
                fn[0] = '_';
                break;
            }
        }

        if(fn.end() - fn.begin() > (std::ptrdiff_t)prefix_length && std::find(fn.begin() + prefix_length, fn.end(), '_') != fn.end())
            fn[0] = '_';
    }

    // sort filenames
    std::sort(list.begin(), list.end());

    const auto langs_begin = std::lower_bound(list.cbegin(), list.cend(), "thextech_");
    const auto langs_end   = std::upper_bound(list.cbegin(), list.cend(), "thextech`");

    if(langs_begin == langs_end)
    {
        pLogDebug("Languages directory does not have thextech_*.json files: %s", langs.absolutePath().c_str());
        return;
    }

    // fill s_languages
    for(auto it = langs_begin; it != langs_end; ++it)
    {
        // check filesize > 8 bytes
        auto f = SDL_RWFromFile((langs.absolutePath() + "/" + *it).c_str(), "rb");

        // file doesn't exist / has problem opening
        if(!f)
            continue;

        auto fsize = SDL_RWsize(f);
        SDL_RWclose(f);

        // file too small
        if(fsize < 8)
            continue;

        s_languages.push_back(*it);
    }
}


void XLanguage::resolveLanguage(const std::string& requestedLanguage)
{
    CurrentLanguage = "";
    CurrentLangDialect = "";

    if(!requestedLanguage.empty() && requestedLanguage != "auto")
    {
        CurrentLanguage = requestedLanguage;

        splitRegion('-');
        splitRegion('_');

        if(detectSetup())
            return; // Found!
    }

#ifndef THEXTECH_DISABLE_SDL_LOCALE
    pLogDebug("Checking SDL localization...");

    SDL_Locale *loc = SDL_GetPreferredLocales();
    CurrentLanguage.clear();
    CurrentLangDialect.clear();

    for(; loc; ++loc)
    {
        if(!loc->language && !loc->country)
            break; // Empty entry

        if(loc->language)
            CurrentLanguage = loc->language;

        if(loc->country)
        {
            CurrentLangDialect = loc->country;
            std::transform(CurrentLangDialect.begin(),
                           CurrentLangDialect.end(),
                           CurrentLangDialect.begin(),
                           [](unsigned char c) { return std::tolower(c); });
        }

        if(detectSetup())
            return; // Found!
    }
#endif

    // Detect using system specific ways
    pLogDebug("Checking system localization...");

    XLanguagePriv::detectOSLanguage();
    if(detectSetup())
        return; // Found!

    pLogDebug("Checking en-gb fallback...");

    // Fall back to English if selected language is invalid
    CurrentLanguage = "en";
    CurrentLangDialect = "gb";

    if(detectSetup())
        return; // Found!

    if(!detectSetup())
    {
        CurrentLanguage.clear();
        CurrentLangDialect.clear();
    }
}

void XLanguage::rotateLanguage(std::string& nextLanguage, int step)
{
    constexpr size_t prefix_length = 9; // std::string("thextech_").size();
    constexpr size_t suffix_length = 5; // std::string(".json").size();
    constexpr size_t ignore_length = prefix_length + suffix_length;

    if(s_languages.empty())
    {
        pLogDebug("Cannot rotate language because none are present");
        return;
    }

    // find current filename
    std::string seek_fn = fmt::format_ne("thextech_{0}.json", g_config.language);

    auto curr_lang_fn = std::find(s_languages.cbegin(), s_languages.cend(), seek_fn);

    if(g_config.language == "auto")
        curr_lang_fn = s_languages.cend();

    // pick next filename
    std::ptrdiff_t cur_index = curr_lang_fn - s_languages.cbegin();
    std::ptrdiff_t new_index = cur_index + step;
    std::ptrdiff_t opts_len  = s_languages.size() + 1;

    // limit to bounds + 1 (in order to include "auto")
    std::ptrdiff_t wrapped_index = (new_index < 0) ? (new_index % opts_len + opts_len) : (new_index % opts_len);

    auto next_lang_fn = s_languages.cbegin() + wrapped_index;

    if(next_lang_fn == s_languages.cend())
    {
        nextLanguage = "auto";
        return;
    }

    // extract next language name
    nextLanguage = next_lang_fn->substr(prefix_length, next_lang_fn->size() - ignore_length);
}

void XLanguage::splitRegion(char delimiter)
{
    if(CurrentLanguage.find(delimiter) == std::string::npos)
        return;

    std::vector<std::string> s;
    Strings::split(s, CurrentLanguage, delimiter);

    if(s.size() >= 2)
    {
        CurrentLanguage = s[0];
        CurrentLangDialect = s[1];
    }
}

const std::string &XLanguage::getEngineFile()
{
    return langEngineFile;
}


const std::string &XLanguage::getAssetsFile()
{
    return langAssetsFile;
}
