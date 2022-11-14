/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2022 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#include <Foundation/Foundation.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_filesystem.h>

#include "app_path_macos_dirs.h"

char * getAppSupportDir(void)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    char *retval = NULL;
    NSString *str;
    const char *base;

    NSArray *array = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);

    if([array count] > 0) /* we only want the first item in the list. */
    {
        str = [array objectAtIndex:0];
        base = [str fileSystemRepresentation];

        if(base)
        {
            const size_t len = SDL_strlen(base) + 4;
            retval = (char *)SDL_malloc(len);

            if(retval == NULL)
                SDL_OutOfMemory();
            else
                SDL_snprintf(retval, len, "%s", base);
        }
    }

    [pool drain];
    return retval;
}

char * getScreenCaptureDir(void)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    char *retval = NULL;
    NSUserDefaults *appUserDefaults;
    NSDictionary *prefsDict;
    NSString *str;
    const char *base;

    /* Get current screencapture location */
    appUserDefaults = [[NSUserDefaults alloc] init];
    [appUserDefaults addSuiteNamed:@"com.apple.screencapture"];
    prefsDict = [appUserDefaults dictionaryRepresentation];

    str = [prefsDict valueForKey:@"location"];
    base = [str fileSystemRepresentation];

    if(base)
    {
        const size_t len = SDL_strlen(base) + 4;
        retval = (char *)SDL_malloc(len);

        if(retval == NULL)
            SDL_OutOfMemory();
        else
            SDL_snprintf(retval, len, "%s", base);
    }

    [pool drain];
    return retval;
}
