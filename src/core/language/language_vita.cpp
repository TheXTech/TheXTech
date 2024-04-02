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

#include "language_private.h"
#include "globals.h"

#include <psp2/apputil.h>
#include <psp2/system_param.h>

#include <Logger/logger.h>


void XLanguagePriv::detectOSLanguage()
{
    Sint32 language = SCE_SYSTEM_PARAM_LANG_ENGLISH_GB;
    SceAppUtilInitParam initParam;
    SceAppUtilBootParam bootParam;
    std::memset(&initParam, 0, sizeof(SceAppUtilInitParam));
    std::memset(&bootParam, 0, sizeof(SceAppUtilBootParam));

    CurrentLanguage.clear();
    CurrentLangDialect.clear();

    sceAppUtilInit(&initParam, &bootParam);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &language);

    switch(language)
    {
    case SCE_SYSTEM_PARAM_LANG_JAPANESE:
        CurrentLanguage    = "ja";
        CurrentLangDialect = "jp";
        break;

    case SCE_SYSTEM_PARAM_LANG_ENGLISH_US:
        CurrentLanguage    = "en";
        CurrentLangDialect = "us";
        break;

    case SCE_SYSTEM_PARAM_LANG_FRENCH:
        CurrentLanguage    = "fr";
        CurrentLangDialect = "fr";
        break;

    case SCE_SYSTEM_PARAM_LANG_SPANISH:
        CurrentLanguage    = "es";
        CurrentLangDialect = "es";
        break;

    case SCE_SYSTEM_PARAM_LANG_GERMAN:
        CurrentLanguage    = "de";
        CurrentLangDialect = "de";
        break;

    case SCE_SYSTEM_PARAM_LANG_ITALIAN:
        CurrentLanguage    = "it";
        CurrentLangDialect = "it";
        break;

    case SCE_SYSTEM_PARAM_LANG_DUTCH:
        CurrentLanguage    = "nl";
        CurrentLangDialect = "nl";
        break;

    case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT:
        CurrentLanguage    = "pt";
        CurrentLangDialect = "pt";
        break;

    case SCE_SYSTEM_PARAM_LANG_RUSSIAN:
        CurrentLanguage    = "ru";
        CurrentLangDialect = "ru";
        break;

    case SCE_SYSTEM_PARAM_LANG_KOREAN:
        CurrentLanguage    = "ko";
        CurrentLangDialect = "kr";
        break;

    case SCE_SYSTEM_PARAM_LANG_CHINESE_T:
        CurrentLanguage    = "zh";
        CurrentLangDialect = "tw";
        break;

    case SCE_SYSTEM_PARAM_LANG_CHINESE_S:
        CurrentLanguage    = "zh";
        CurrentLangDialect = "cn";
        break;

    case SCE_SYSTEM_PARAM_LANG_FINNISH:
        CurrentLanguage    = "fi";
        CurrentLangDialect = "fi";
        break;

    case SCE_SYSTEM_PARAM_LANG_SWEDISH:
        CurrentLanguage    = "sv";
        CurrentLangDialect = "se";
        break;

    case SCE_SYSTEM_PARAM_LANG_DANISH:
        CurrentLanguage    = "da";
        CurrentLangDialect = "dk";
        break;

    case SCE_SYSTEM_PARAM_LANG_NORWEGIAN:
        CurrentLanguage    = "nb";
        CurrentLangDialect = "no";
        break;

    case SCE_SYSTEM_PARAM_LANG_POLISH:
        CurrentLanguage    = "pl";
        CurrentLangDialect = "pl";
        break;

    case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR:
        CurrentLanguage    = "pt";
        CurrentLangDialect = "br";
        break;

    case SCE_SYSTEM_PARAM_LANG_ENGLISH_GB:
    default:
        CurrentLanguage    = "en";
        CurrentLangDialect = "gb";
        break;

    case SCE_SYSTEM_PARAM_LANG_TURKISH:
        CurrentLanguage    = "tr";
        CurrentLangDialect = "tr";
        break;
    }


    pLogDebug("Vita: Detected syetem language: %s-%s",
              CurrentLanguage.c_str(),
              CurrentLangDialect.c_str());
}
