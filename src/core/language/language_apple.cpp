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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <string>

#include "language_private.h"

// Duplicate from globals.h due to conflict with system-wide headers
extern std::string CurrentLanguage;
extern std::string CurrentLangDialect;


void XLanguagePriv::detectOSLanguage()
{
    CurrentLanguage.clear();
    CurrentLangDialect.clear();

    CFLocaleRef cflocale = CFLocaleCopyCurrent();

    CFStringRef lang_value = (CFStringRef)CFLocaleGetValue(cflocale, kCFLocaleLanguageCode);
    CFStringRef country_value = (CFStringRef)CFLocaleGetValue(cflocale, kCFLocaleCountryCode);

    CFIndex lang_length  = CFStringGetLength(lang_value);
    CFIndex country_length  = CFStringGetLength(country_value);

    CFIndex maxSizeLang = CFStringGetMaximumSizeForEncoding(lang_length, kCFStringEncodingUTF8) + 1;
    CFIndex maxSizeCoun = CFStringGetMaximumSizeForEncoding(country_length, kCFStringEncodingUTF8) + 1;

    CFIndex maxSize = std::max(maxSizeLang, maxSizeCoun);

    char *buffer = (char*)std::malloc(maxSize);

    if(CFStringGetCString(lang_value, buffer, maxSize, kCFStringEncodingUTF8))
        CurrentLanguage = std::string(buffer);
    else
    {
        std::fprintf(stderr, "COCOA: Failed to retreive language code");
        std::fflush(stderr);
    }

    if(CFStringGetCString(country_value, buffer, maxSize, kCFStringEncodingUTF8))
    {
        CurrentLangDialect = std::string(buffer);
        std::transform(CurrentLangDialect.begin(),
                       CurrentLangDialect.end(),
                       CurrentLangDialect.begin(),
                       [](unsigned char c) { return std::tolower(c); });
    }
    else
    {
        std::fprintf(stderr, "COCOA: Failed to retreive country code");
        std::fflush(stderr);
    }

    std::free(buffer);
    CFRelease(cflocale);
}
