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

#include "language_private.h"
#include "globals.h"

#include <psputility.h>

#include <Logger/logger.h>


void XLanguagePriv::detectOSLanguage()
{
    int language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;

    CurrentLanguage.clear();
    CurrentLangDialect.clear();

    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &language);

    switch(language)
    {
    case PSP_SYSTEMPARAM_LANGUAGE_JAPANESE:
        CurrentLanguage    = "ja";
        CurrentLangDialect = "jp";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_ENGLISH:
    default:
        CurrentLanguage    = "en";
        CurrentLangDialect = "gb";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_FRENCH:
        CurrentLanguage    = "fr";
        CurrentLangDialect = "fr";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_SPANISH:
        CurrentLanguage    = "es";
        CurrentLangDialect = "es";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_GERMAN:
        CurrentLanguage    = "de";
        CurrentLangDialect = "de";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_ITALIAN:
        CurrentLanguage    = "it";
        CurrentLangDialect = "it";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_DUTCH:
        CurrentLanguage    = "nl";
        CurrentLangDialect = "nl";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_PORTUGUESE:
        CurrentLanguage    = "pt";
        CurrentLangDialect = "pt";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_RUSSIAN:
        CurrentLanguage    = "ru";
        CurrentLangDialect = "ru";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_KOREAN:
        CurrentLanguage    = "ko";
        CurrentLangDialect = "kr";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_CHINESE_TRADITIONAL:
        CurrentLanguage    = "zh";
        CurrentLangDialect = "tw";
        break;

    case PSP_SYSTEMPARAM_LANGUAGE_CHINESE_SIMPLIFIED:
        CurrentLanguage    = "zh";
        CurrentLangDialect = "cn";
        break;
    }


    pLogDebug("PSP: Detected syetem language: %s-%s",
              CurrentLanguage.c_str(),
              CurrentLangDialect.c_str());
}
