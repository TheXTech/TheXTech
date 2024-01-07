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

#include <cstdlib>
#include <algorithm>

#include <windows.h>

#include "globals.h"
#include "language_private.h"
#include "../language.h"


void XLanguagePriv::detectOSLanguage()
{
    CurrentLanguage.clear();
    CurrentLangDialect.clear();

    LCID locale = GetSystemDefaultLCID();
    char langStr[21];

    std::memset(langStr, 0, 21);
    GetLocaleInfoA(locale, LOCALE_SISO639LANGNAME, langStr, 20);
    CurrentLanguage.clear();
    CurrentLanguage.append(langStr);

    std::memset(langStr, 0, 21);
    GetLocaleInfoA(locale, LOCALE_SISO3166CTRYNAME, langStr, 20);
    CurrentLangDialect.clear();
    CurrentLangDialect.append(langStr);

    std::transform(CurrentLangDialect.begin(),
                   CurrentLangDialect.end(),
                   CurrentLangDialect.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}
