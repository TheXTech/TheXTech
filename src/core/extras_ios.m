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
#include <UIKit/UIWindow.h>
#include <SDL2/SDL_video.h>
#include <sys/utsname.h>

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
// Fore iOS 13+
#   include <CoreHaptics/CoreHaptics.h>
#endif
// For iOS older than 13
#include <AudioToolbox/AudioToolbox.h>

/* Copy from private heads of SDL2 (src/video/uikit/SDL_uikitappdelegate.h) to inherit */
@interface SDLUIKitDelegate : NSObject<UIApplicationDelegate>

+ (id)sharedAppDelegate;
+ (NSString *)getAppDelegateClassName;

- (void)hideLaunchScreen;

/* This property is marked as optional, and is only intended to be used when
 * the app's UI is storyboard-based. SDL is not storyboard-based, however
 * several major third-party ad APIs (e.g. Google admob) incorrectly assume this
 * property always exists, and will crash if it doesn't. */
@property (nonatomic) UIWindow *window;
@end


@interface TheXTechDelegate : SDLUIKitDelegate
- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer;
@end

@implementation TheXTechDelegate
- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer
{
    return NO; /* Workaround to not steal internal hesture handler */
}
@end


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


/* Haptics Implementation */

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
static int s_hapticsSupported = -1;
static int s_hapticsCounter = 0;

API_AVAILABLE(ios(13.0))
static CHHapticEngine *s_hapticsEngine = nil;

#endif

int ios_vibrator_init()
{
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
    NSError *error = nil;

    if(@available(iOS 13.0, *))
    {
        if(s_hapticsEngine)
        {
            ++s_hapticsCounter;
            return s_hapticsSupported;
        }
        
        s_hapticsSupported = -1;

        if(![CHHapticEngine capabilitiesForHardware].supportsHaptics)
            return -1; /* Hardware does not supports haptics */

        s_hapticsEngine = [[CHHapticEngine alloc] initAndReturnError:&error];
        if(error)
            return -1;
        
        s_hapticsSupported = 0;
        ++s_hapticsCounter;
        
        [s_hapticsEngine setResetHandler:^
        {
            NSLog(@"Haptics Engine RESET!");
                
            // Try restarting the engine again.
            NSError* startupError;
            [s_hapticsEngine startAndReturnError:&startupError];
            
            if(startupError)
                NSLog(@"ERROR: Haptics Engine couldn't restart!");
            
            // Register any custom resources you had registered, using registerAudioResource.
            // Recreate all haptic pattern players you had created, using createPlayer.
        }];
        
        [s_hapticsEngine setStoppedHandler:^(CHHapticEngineStoppedReason reason)
        {
            switch(reason)
            {
            default:
                break;

            case CHHapticEngineStoppedReasonAudioSessionInterrupt:
            {
                NSLog(@"REASON: Audio Session Interrupt");
                break;
            }
            case CHHapticEngineStoppedReasonApplicationSuspended:
            {
                NSLog(@"REASON: Application Suspended");
                break;
            }
            
            case CHHapticEngineStoppedReasonIdleTimeout:
            {
                NSLog(@"REASON: Idle Timeout");
                break;
            }
            
            case CHHapticEngineStoppedReasonSystemError:
            {
                NSLog(@"REASON: System Error");
                break;
            }
            }
        }];

        return 0;
    }
#endif
    return -1;
}

int ios_vibrator_quit()
{
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
    if(@available(iOS 13.0, *))
    {
        if(s_hapticsEngine)
        {
            if(s_hapticsCounter-- == 1)
            {
                s_hapticsEngine = nil;
                [s_hapticsEngine release];
                s_hapticsSupported = -1;
            }
        }
    }
#endif
}

void ios_trigger_vibrator(float strenght, int ms)
{
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
    if(ios_trigger_vibrator_taps(strenght, ms) == 0)
        return;
#else
    (void)strenght;
#endif

    /* Fallback for devices without haptics, and for iOS older than 13! */
    if(ms >= 50)
        AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
    else if(ms >= 12)
        AudioServicesPlaySystemSound(1520);
    else
        AudioServicesPlaySystemSound(1519);
}

int ios_trigger_vibrator_taps(float strenght, int ms)
{
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
    if(@available(iOS 13.0, *))
    {
        NSError* error;

        if(s_hapticsSupported == 0)
        {
            CHHapticEventParameter *strengthParameter = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticIntensity value:strenght];
            CHHapticEventParameter *sharpnessParameter = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticSharpness value:1.0];
            
            CHHapticEvent *event = [[CHHapticEvent alloc] initWithEventType:CHHapticEventTypeHapticContinuous parameters:@[strengthParameter, sharpnessParameter] relativeTime:0 duration:(ms / 1000.0f)];
            CHHapticPattern *patten = [[CHHapticPattern alloc] initWithEvents:@[event] parameterCurves:@[] error:&error];

            id<CHHapticPatternPlayer> player = [s_hapticsEngine createPlayerWithPattern:patten error:&error];
            
            [s_hapticsEngine startAndReturnError:&error];
            [player startAtTime:0 error:&error];
            
            // FIXME: An error occurs:
            // CHHapticEngine.mm:1917  -[CHHapticEngine(CHHapticEngineInternal) getAvailableChannel:]: ERROR: Unable to add an additional player channel
            // after several time handling of the vibration. It starts to fail.

            return 0;
        }
    }
#else
    (void)strenght;
#endif

    if(ms >= 12)
        AudioServicesPlaySystemSound(1520);
    else if(ms >= 6)
        AudioServicesPlaySystemSound(1519);
    else
        return -1;

    return 0;
}
