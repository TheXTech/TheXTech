/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2026 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "extras.h"

#include <tgmath.h>
#include <Foundation/Foundation.h>
#include <UIKit/UIApplication.h>
#include <UIKit/UIScreen.h>
#include <SDL2/SDL_video.h>

void ios_quit(int ret)
{
    //home button press programmatically
    UIApplication *app = [UIApplication sharedApplication];
    [app performSelector:@selector(suspend)];

    //wait 2 seconds while app is going background
    [NSThread sleepForTimeInterval:2.0];

    //exit app when app is in background
    exit(ret);
}

double ios_get_screen_diagonal(double *ow, double *oh)
{
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    CGFloat scale = UIScreen.mainScreen.scale;
    CGFloat w = screenRect.size.width * scale;
    CGFloat h = screenRect.size.height * scale;
    float diagDPI = -1;
    float horiDPI = -1;
    float vertDPI = -1;
    double diag;
    
    *ow = w;
    *oh = h;
    
    // SDL2 already returns the DPI that can be used to compute physical thing
    SDL_GetDisplayDPI(0, &diagDPI, &horiDPI, &vertDPI);
    
    // Diagonal - is a hypotinuse!
    diag = sqrt((w * w) + (h * h)) / diagDPI;

    return diag;
}

