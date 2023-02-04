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

#include <algorithm>
#include <Utils/strings.h>
#include <Utils/files.h>
#include <AppPath/app_path.h>
#include "language_private.h"
#include <fmt_format_ne.h>

#ifndef XTECH_DISABLE_SDL_LOCALE
#   include <SDL2/SDL_version.h>
#   if SDL_COMPILEDVERSION >= SDL_VERSIONNUM(2, 0, 14)
#       include <SDL2/SDL_locale.h>
#   else
#       define XTECH_DISABLE_SDL_LOCALE
#   endif
#endif

#include "globals.h"
#include "../language.h"


static std::string langEngineFile;
static std::string langAssetsFile;


static bool detectSetup()
{
    if(CurrentLanguage.empty())
        return false; // Language code is required!

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

    return !langEngineFile.empty() || !langAssetsFile.empty();
}


void XLanguage::init()
{
#ifndef XTECH_DISABLE_SDL_LOCALE
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
    XLanguagePriv::detectOSLanguage();
    detectSetup();
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
