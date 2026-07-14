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
#include <sys/utsname.h>

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

int ios_get_cut_off_size(void)
{
    NSDictionary* devices;
    struct utsname systemInfo;
    NSString* deviceName;
    int ret = 0;
    id foundCut;

    /*
     * A well up to date list of device info can be found here:
     * https://github.com/lmirosevic/GBDeviceInfo/blob/master/GBDeviceInfo/GBDeviceInfo_iOS.m
     */
    devices = @{
        // X
        @"iPhone10,3": @250,
        @"iPhone10,6": @250,
        // XR
        @"iPhone11,8": @250,
        // XS
        @"iPhone11,2": @250,
        // XS Max
        @"iPhone11,4": @250,
        @"iPhone11,6": @250,
        // 11
        @"iPhone12,1": @250,
        // 11 Pro
        @"iPhone12,3": @250,
        // 11 Pro Max
        @"iPhone12,5": @250,
        // 12 mini
        @"iPhone13,1": @250,
        // 12
        @"iPhone13,2": @250,
        // 12 Pro
        @"iPhone13,3": @250,
        // 12 Pro Max
        @"iPhone13,4": @250,
        // 13 mini
        @"iPhone14,4": @250,
        // 13
        @"iPhone14,5": @250,
        // 13 Pro
        @"iPhone14,2": @250,
        // 13 Pro Max
        @"iPhone14,3": @250,
        // 14
        @"iPhone14,7": @250,
        // 14 Plus
        @"iPhone14,8": @250,

        // These devices now have different cut off

        // 14 Pro
        @"iPhone15,2": @270,
        // 14 Pro Max
        @"iPhone15,3": @270,
        // 15
        @"iPhone15,4": @270,
        // 15 Plus
        @"iPhone15,5": @270,
        // 15 Pro
        @"iPhone16,1": @270,
        // 15 Pro Max
        @"iPhone16,2": @270,
        // 16 Pro
        @"iPhone17,1": @270,
        // 16 Pro Max
        @"iPhone17,2": @270,
        // 16
        @"iPhone17,3": @270,
        // 16 Plus
        @"iPhone17,4": @270,
        // 17
        @"iPhone18,3": @270,
        // Air
        @"iPhone18,4": @270,
        // 17 Pro
        @"iPhone18,1": @270,
        // 17 Pro Max
        @"iPhone18,2": @270,
        // 17e
        @"iPhone18,5": @270,
    };

    uname(&systemInfo);
    deviceName = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
    foundCut = devices[deviceName];
    if(foundCut)
        ret = (int)[foundCut integerValue];

    return ret;
}
