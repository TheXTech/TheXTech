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

#include "language_private.h"
#include "globals.h"
#include <algorithm>
#include <3ds.h>


void XLanguagePriv::detectOSLanguage()
{
    u8 language = 0;
    u8 region = 0;
    Result res;

    CurrentLanguage.clear();
    CurrentLangDialect.clear();

    cfguInit();

    // Read the language field from the config savegame.
    res = CFGU_GetSystemLanguage(&language);
    if(res != 0)
    {
        std::fprintf(stderr, "CFGU_GetSystemLanguage() failed: 0x%x.\n", (unsigned int)res);
        std::fflush(stderr);
        cfguExit();
        return;
    }

    res = CFGU_SecureInfoGetRegion(&region);
    if(res != 0)
    {
        std::fprintf(stderr, "CFGU_SecureInfoGetRegion() failed: 0x%x.\n", (unsigned int)res);
        std::fflush(stderr);
        cfguExit();
        return;
    }

    cfguExit();

    switch(language)
    {
    case CFG_LANGUAGE_JP:
        CurrentLanguage = "ja";
        CurrentLangDialect = "jp";
        break;

    default:
    case CFG_LANGUAGE_EN:
        CurrentLanguage = "en";
        switch(region)
        {
        case CFG_REGION_USA:
            CurrentLangDialect = "us";
            break;
        case CFG_REGION_AUS:
            CurrentLangDialect = "au";
            break;
        default:
        case CFG_REGION_EUR:
            CurrentLangDialect = "gb";
            break;
        }
        break;

    case CFG_LANGUAGE_FR:
        CurrentLanguage = "fr";
        switch(region)
        {
        case CFG_REGION_USA:
            CurrentLangDialect = "ca";
            break;
        default:
        case CFG_REGION_EUR:
            CurrentLangDialect = "fr";
            break;
        }
        break;

    case CFG_LANGUAGE_DE:
        CurrentLanguage = "de";
        CurrentLangDialect = "de";
        break;

    case CFG_LANGUAGE_IT:
        CurrentLanguage = "it";
        CurrentLangDialect = "it";
        break;

    case CFG_LANGUAGE_ES:
        CurrentLanguage = "es";
        switch(region)
        {
        case CFG_REGION_USA:
            CurrentLangDialect = "mx";
            break;
        default:
        case CFG_REGION_EUR:
            CurrentLangDialect = "es";
            break;
        }
        break;

    case CFG_LANGUAGE_ZH:
        CurrentLanguage = "zh";
        CurrentLangDialect = "cn";
        break;

    case CFG_LANGUAGE_KO:
        CurrentLanguage = "ko";
        CurrentLangDialect = "ko";
        break;

    case CFG_LANGUAGE_NL:
        CurrentLanguage = "nl";
        CurrentLangDialect = "nl";
        break;

    case CFG_LANGUAGE_PT:
        CurrentLanguage = "pt";
        switch(region)
        {
        case CFG_REGION_USA:
            CurrentLangDialect = "br";
            break;
        default:
        case CFG_REGION_EUR:
            CurrentLangDialect = "pt";
            break;
        }
        break;

    case CFG_LANGUAGE_RU:
        CurrentLanguage = "ru";
        CurrentLangDialect = "ru";
        break;

    case CFG_LANGUAGE_TW:
        CurrentLanguage = "zh";
        CurrentLangDialect = "tw";
        break;
    }

    std::fprintf(stderr, "Detected syetem language: %s-%s\n",
                 CurrentLanguage.c_str(),
                 CurrentLangDialect.c_str());
    std::fflush(stderr);
}
