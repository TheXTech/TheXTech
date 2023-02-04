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
#include <switch.h>


void XLanguagePriv::detectOSLanguage()
{
    u64 LanguageCode = 0;
    SetLanguage Language = SetLanguage_ENGB;

    CurrentLanguage.clear();
    CurrentLangDialect.clear();

    Result rc = setInitialize();
    if(R_FAILED(rc))
    {
        std::fprintf(stderr, "setInitialize() failed: 0x%x.\n", rc);
        std::fflush(stderr);
        return;
    }

    rc = setGetSystemLanguage(&LanguageCode);
    if(R_FAILED(rc))
    {
        std::fprintf(stderr, "setGetSystemLanguage() failed: 0x%x.\n", rc);
        std::fflush(stderr);
        setExit();
        return;
    }

    rc = setMakeLanguage(LanguageCode, &Language);
    if(R_FAILED(rc))
    {
        std::fprintf(stderr, "setMakeLanguage() failed: 0x%x.\n", rc);
        std::fflush(stderr);
        setExit();
        return;
    }

    setExit();

    switch(Language)
    {
    case SetLanguage_JA:
        CurrentLanguage = "ja";
        CurrentLangDialect = "jp";
        break;

    case SetLanguage_ENUS:
        CurrentLanguage = "en";
        CurrentLangDialect = "us";
        break;

    case SetLanguage_FR:
        CurrentLanguage = "fr";
        CurrentLangDialect = "fr";
        break;

    case SetLanguage_DE:
        CurrentLanguage = "de";
        CurrentLangDialect = "de";
        break;

    case SetLanguage_IT:
        CurrentLanguage = "it";
        CurrentLangDialect = "it";
        break;

    case SetLanguage_ES:
        CurrentLanguage = "es";
        CurrentLangDialect = "es";
        break;

    case SetLanguage_ZHCN:
    case SetLanguage_ZHHANS:
        CurrentLanguage = "zh";
        CurrentLangDialect = "cn";
        break;

    case SetLanguage_KO:
        CurrentLanguage = "ko";
        CurrentLangDialect = "ko";
        break;

    case SetLanguage_NL:
        CurrentLanguage = "nl";
        CurrentLangDialect = "nl";
        break;

    case SetLanguage_PT:
        CurrentLanguage = "pt";
        CurrentLangDialect = "pt";
        break;

    case SetLanguage_RU:
        CurrentLanguage = "ru";
        CurrentLangDialect = "ru";
        break;

    case SetLanguage_ZHTW:
    case SetLanguage_ZHHANT:
        CurrentLanguage = "zh";
        CurrentLangDialect = "tw";
        break;

    default:
    case SetLanguage_ENGB:
        CurrentLanguage = "en";
        CurrentLangDialect = "gb";
        break;

    case SetLanguage_FRCA:
        CurrentLanguage = "fr";
        CurrentLangDialect = "ca";
        break;

    case SetLanguage_ES419:
        CurrentLanguage = "es";
        CurrentLangDialect = "mx";
        break;

    case SetLanguage_PTBR:
        CurrentLanguage = "pt";
        CurrentLangDialect = "br";
        break;
    }

    std::fprintf(stderr, "Detected syetem language: %s-%s\n",
                 CurrentLanguage.c_str(),
                 CurrentLangDialect.c_str());
    std::fflush(stderr);
}
