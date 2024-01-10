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

#include <swkbd/rpl_interface.h>

#include <coreinit/memdefaultheap.h>
#include <coreinit/userconfig.h>

#include <Logger/logger.h>


void XLanguagePriv::detectOSLanguage()
{
    UCError err;
    nn::swkbd::LanguageType language = nn::swkbd::LanguageType::English;
    nn::swkbd::RegionType region = nn::swkbd::RegionType::Europe;

    CurrentLanguage.clear();
    CurrentLangDialect.clear();

    UCHandle handle = UCOpen();
    if(handle < 1)
    {
        CurrentLanguage = "en";
        CurrentLangDialect = "gb";
    }

    UCSysConfig *settings = (UCSysConfig*)MEMAllocFromDefaultHeapEx(sizeof(UCSysConfig), 0x40);
    if(!settings)
    {
        UCClose(handle);
        CurrentLanguage = "en";
        CurrentLangDialect = "gb";

        pLogWarning("WiiU: Failed to open settings instance, falling back to: %s-%s",
                    CurrentLanguage.c_str(),
                    CurrentLangDialect.c_str());

        return;
    }

    strcpy(settings->name, "cafe.language");
    settings->access = 0;
    settings->dataType = UC_DATATYPE_UNSIGNED_INT;
    settings->error = UC_ERROR_OK;
    settings->dataSize = sizeof(nn::swkbd::LanguageType);
    settings->data = &language;

    err = UCReadSysConfig(handle, 1, settings);

    if(err == UC_ERROR_OK)
    {
        strcpy(settings->name, "cafe.region");
        settings->dataSize = sizeof(nn::swkbd::LanguageType);
        settings->data = &region;
        err = UCReadSysConfig(handle, 1, settings);
    }

    UCClose(handle);
    MEMFreeToDefaultHeap(settings);

    if(err != UC_ERROR_OK)
    {
        CurrentLanguage = "en";
        CurrentLangDialect = "gb";

        pLogWarning("WiiU: Failed to obtain system language, falling back to: %s-%s",
                  CurrentLanguage.c_str(),
                  CurrentLangDialect.c_str());

        return;
    }

    switch(language)
    {
    case nn::swkbd::LanguageType::Japanese:
        CurrentLanguage = "ja";
        CurrentLangDialect = "jp";
        break;

    default:
    case nn::swkbd::LanguageType::English:
        CurrentLanguage = "en";
        switch(region)
        {
        case nn::swkbd::RegionType::USA:
            CurrentLangDialect = "us";
            break;
        default:
        case nn::swkbd::RegionType::Europe:
            CurrentLangDialect = "gb";
            break;
        }
        break;

    case nn::swkbd::LanguageType::French:
        CurrentLanguage = "fr";
        switch(region)
        {
        case nn::swkbd::RegionType::USA:
            CurrentLangDialect = "ca";
            break;
        default:
        case nn::swkbd::RegionType::Europe:
            CurrentLangDialect = "fr";
            break;
        }
        break;

    case nn::swkbd::LanguageType::German:
        CurrentLanguage = "de";
        CurrentLangDialect = "de";
        break;

    case nn::swkbd::LanguageType::Italian:
        CurrentLanguage = "it";
        CurrentLangDialect = "it";
        break;

    case nn::swkbd::LanguageType::Spanish:
        CurrentLanguage = "es";
        switch(region)
        {
        case nn::swkbd::RegionType::USA:
            CurrentLangDialect = "mx";
            break;
        default:
        case nn::swkbd::RegionType::Europe:
            CurrentLangDialect = "es";
            break;
        }
        break;

    case static_cast<nn::swkbd::LanguageType>(6): // nn::swkbd::LanguageType::SimplifiedChinese: // FIXME: Workaround for WUT's bug (Chinese Traditional and Simplified are swapped)
        CurrentLanguage = "zh";
        CurrentLangDialect = "cn";
        break;

    case nn::swkbd::LanguageType::Korean:
        CurrentLanguage = "ko";
        CurrentLangDialect = "ko";
        break;

    case nn::swkbd::LanguageType::Dutch:
        CurrentLanguage = "nl";
        CurrentLangDialect = "nl";
        break;

    case nn::swkbd::LanguageType::Portuguese:
        CurrentLanguage = "pt";
        switch(region)
        {
        case nn::swkbd::RegionType::USA:
            CurrentLangDialect = "br";
            break;
        default:
        case nn::swkbd::RegionType::Europe:
            CurrentLangDialect = "pt";
            break;
        }
        break;

    case nn::swkbd::LanguageType::Russian:
        CurrentLanguage = "ru";
        CurrentLangDialect = "ru";
        break;

    case static_cast<nn::swkbd::LanguageType>(11): //nn::swkbd::LanguageType::TraditionalChinese: // FIXME: Workaround for WUT's bug (Chinese Traditional and Simplified are swapped)
        CurrentLanguage = "zh";
        CurrentLangDialect = "tw";
        break;
    }

    pLogDebug("WiiU: Detected syetem language: %s-%s",
              CurrentLanguage.c_str(),
              CurrentLangDialect.c_str());
}
