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

#include <jni.h>
#if 1
#   undef JNIEXPORT
#   undef JNICALL
#   define JNIEXPORT extern "C"
#   define JNICALL
#endif

static std::string s_lang;
static std::string s_country;

JNIEXPORT void JNICALL
Java_ru_wohlsoft_thextech_thextechActivity_setLanguageCodes(
    JNIEnv* env,
    jclass clazz,
    jstring lang_j,
    jstring country_j)
{
    const char *line;
    (void)clazz;
    line = env->GetStringUTFChars(lang_j, nullptr);
    s_lang = line;
    env->ReleaseStringUTFChars(lang_j, line);

    line = env->GetStringUTFChars(country_j, nullptr);
    s_country = line;
    env->ReleaseStringUTFChars(country_j, line);
}


void XLanguagePriv::detectOSLanguage()
{
    CurrentLanguage = s_lang;
    CurrentLangDialect = s_country;
    std::transform(CurrentLanguage.begin(),
                   CurrentLanguage.end(),
                   CurrentLanguage.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}
