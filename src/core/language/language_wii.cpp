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

#include <ogc/conf.h>

#include <Logger/logger.h>


void XLanguagePriv::detectOSLanguage()
{
    CONF_Init();

    u32 language = CONF_GetLanguage();
    u32 region = CONF_GetRegion();

    CurrentLanguage.clear();
    CurrentLangDialect.clear();

    switch(language)
    {
    case CONF_LANG_JAPANESE:
        CurrentLanguage = "ja";
        CurrentLanguage = "jp";
        break;

    default:
    case CONF_LANG_ENGLISH:
        CurrentLanguage = "en";
        switch(region)
        {
        case CONF_REGION_US:
            CurrentLangDialect = "us";
            break;
        default:
        case CONF_REGION_EU:
            CurrentLangDialect = "gb";
            break;
        }
        break;

    case CONF_LANG_GERMAN:
        CurrentLanguage = "de";
        CurrentLangDialect = "de";
        break;

    case CONF_LANG_FRENCH:
        CurrentLanguage = "fr";
        switch(region)
        {
        case CONF_REGION_US:
            CurrentLangDialect = "ca";
            break;
        default:
        case CONF_REGION_EU:
            CurrentLangDialect = "fr";
            break;
        }
        break;

    case CONF_LANG_ITALIAN:
        CurrentLanguage = "it";
        CurrentLangDialect = "it";
        break;

    case CONF_LANG_SPANISH:
        CurrentLanguage = "es";
        switch(region)
        {
        case CONF_REGION_US:
            CurrentLangDialect = "mx";
            break;
        default:
        case CONF_REGION_EU:
            CurrentLangDialect = "es";
            break;
        }
        break;

    case CONF_LANG_SIMP_CHINESE:
        CurrentLanguage = "zh";
        CurrentLangDialect = "cn";
        break;

    case CONF_LANG_KOREAN:
        CurrentLanguage = "ko";
        CurrentLangDialect = "ko";
        break;

    case CONF_LANG_DUTCH:
        CurrentLanguage = "nl";
        CurrentLangDialect = "nl";
        break;

    case CONF_LANG_TRAD_CHINESE:
        CurrentLanguage = "zh";
        CurrentLangDialect = "tw";
        break;
    }

    pLogDebug("Wii: Detected syetem language: %s-%s",
              CurrentLanguage.c_str(),
              CurrentLangDialect.c_str());
}
