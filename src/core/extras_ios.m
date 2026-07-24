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
#include <Logger/logger.h>

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
@property (nonatomic, strong) UIWindow *window;
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
    NSDictionary* devices;
    struct utsname systemInfo;
    NSString* deviceName;
    double diag = 6.3; // Average by possible new devices
    id foundDiag;
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    CGFloat scale = UIScreen.mainScreen.scale;
    CGFloat w = screenRect.size.width * scale;
    CGFloat h = screenRect.size.height * scale;

    /*
     * A well up to date list of device info can be found here:
     * https://github.com/lmirosevic/GBDeviceInfo/blob/master/GBDeviceInfo/GBDeviceInfo_iOS.m
     */
    devices =
    @{
        // iPhone 2G
        @"iPhone1,1": @3.5,
        // iPhone 3G
        @"iPhone1,2": @3.5,
        // iPhone 4GS
        @"iPhone2,1": @3.5,
        // iPhone 4 GMS
        @"iPhone3,1": @3.5,
        // iPhone 4 GMS
        @"iPhone3,2": @3.5,
        // iPhone 4 CDMA
        @"iPhone3,3": @3.5,
        // iPhone 4S
        @"iPhone4,1": @3.5,

        // iPhone 5 International
        @"iPhone5,1": @4.0,
        // iPhone 5 China
        @"iPhone5,2": @4.0,
        // iPhone 5c GSM/CDMA
        @"iPhone5,3": @4.0,
        // iPhone 5c CDMA
        @"iPhone5,4": @4.0,
        // iPhone 5s GSM
        @"iPhone6,1": @4.0,
        // iPhone 5s GSM/CDMA
        @"iPhone6,2": @4.0,

        // iPhone 6 Plus
        @"iPhone7,1": @5.5,
        // iPhonet 6
        @"iPhone7,2": @4.7,
        // iPhone 6s
        @"iPhone8,1": @4.7,
        // iPhone 6 Plus
        @"iPhone8,2": @5.5,
        // iPhone SE 1
        @"iPhone8,4": @4.0,

        // iPhone 7
        @"iPhone9,1": @4.7,
        @"iPhone9,3": @4.7,

        // iPhone 7 Plus
        @"iPhone9,2": @5.5,
        @"iPhone9,4": @5.5,

        // iPhone 8
        @"iPhone10,1": @4.7,
        @"iPhone10,4": @4.7,

        // iPhone 8 Plus
        @"iPhone10,2": @5.5,
        @"iPhone10,5": @5.5,

        // iPhone X
        @"iPhone10,3": @5.8,
        @"iPhone10,6": @5.8,

        // iPhone XS
        @"iPhone11,2": @5.8,
        // ????
        @"iPhone11,4": @5.8,

        // iPhone XS Max
        @"iPhone11,6": @6.5,

        // iPhone XR
        @"iPhone11,8": @6.1,

        // iPhone 11
        @"iPhone12,1": @6.1,
        // iPhone 11 Pro
        @"iPhone12,3": @5.8,
        // iPhone 11 Pro Max
        @"iPhone12,5": @6.5,

        // iPhone SE 2
        @"iPhone12,8": @4.7,

        // iPhone 12 Mini
        @"iPhone13,1": @5.4,
        // iPhone 12
        @"iPhone13,2": @6.1,
        // iPhone 12 Pro
        @"iPhone13,3": @6.1,
        // iPhone 12 Pro Max
        @"iPhone13,4": @6.7,

        // iPhone 13 Pro
        @"iPhone14,2": @6.1,
        // iPhone 13 Pro Max
        @"iPhone14,3": @6.7,
        // iPhone 13 Mini
        @"iPhone14,4": @5.4,
        // iPhone 13
        @"iPhone14,5": @6.1,

        // iPhone SE 3
        @"iPhone14,6": @4.7,

        // iPhone 14
        @"iPhone14,7": @6.1,
        // iPhone 14 Plus
        @"iPhone14,8": @6.7,
        // iPhone 14 Pro
        @"iPhone15,2": @6.1,
        // iPhone 14 Pro Max
        @"iPhone15,3": @6.7,

        // iPhone 15
        @"iPhone15,4": @6.1,
        // iPhone 15 Plus
        @"iPhone15,5": @6.7,
        // iPhone 15 Pro
        @"iPhone16,1": @6.1,
        // iPhone 15 Pro Max
        @"iPhone16,2": @6.7,

        // iPhone 16 Pro
        @"iPhone17,1": @6.3,
        // iPhone 16 Pro Max
        @"iPhone17,2": @6.9,
        // iPhone 16
        @"iPhone17,3": @6.1,
        // iPhone 16 Plus
        @"iPhone17,4": @6.7,
        // iPhone 16e
        @"iPhone17,5": @6.1,

        // iPhone 17 Pro
        @"iPhone18,1": @6.3,
        // iPhone 17 Pro Max
        @"iPhone18,2": @6.9,
        // iPhone 17
        @"iPhone18,3": @6.3,
        // iPhone Air
        @"iPhone18,4": @6.5,
        // iPhone 17e
        @"iPhone18,5": @6.1,


        // iPad 1 WiFi/GSM/CDMA
        @"iPad1,1":  @9.7,

        // iPad 2 WiFi
        @"iPad2,1":  @9.7,
        // iPad 2 WiFi + 3G GSM
        @"iPad2,2":  @9.7,
        // iPad 2 WiFi + 3G CDMA
        @"iPad2,3":  @9.7,
        // iPad 2 WiFi
        @"iPad2,4":  @9.7,

        // iPad Mini WiFi
        @"iPad2,5":  @7.9,
        // iPad Mini WiFi + Cell
        @"iPad2,6":  @7.9,
        // iPad Mini WiFi + Cell MM
        @"iPad2,7":  @7.9,

        // iPad 3 WiFi
        @"iPad3,1":  @9.7,
        // iPad 3 WiFi + Cell Verizon
        @"iPad3,2":  @9.7,
        // iPad 3 WiFi + Cell
        @"iPad3,3":  @9.7,
        // iPad 4 WiFi
        @"iPad3,4":  @9.7,
        // iPad 4 WiFi + Cell
        @"iPad3,5":  @9.7,
        // iPad 4 Wi-Fi Cell MM
        @"iPad3,6":  @9.7,

        // iPad Air WiFi
        @"iPad4,1":  @9.7,
        // iPad Air WiFi + Cell
        @"iPad4,2":  @9.7,
        @"iPad4,3":  @9.7,

        // iPad Mini 2 WiFi
        @"iPad4,4":  @7.9,
        // iPad Mini 2 WiFi + Cell
        @"iPad4,5":  @7.9,

        @"iPad4,6":  @7.9,

        // iPad Mini 3 WiFi + Cell
        @"iPad4,7":  @7.9,
        // iPad Mini 3 WiFi + Cell
        @"iPad4,8":  @7.9,

        @"iPad4,9":  @7.9,

        // iPad mini 4 WiFi
        @"iPad5,1":  @7.9,
        // iPad mini 4 WiFi + Cell
        @"iPad5,2":  @7.9,

        // iPad Air 2 WiFi
        @"iPad5,3":  @9.7,
        // iPad Air 2 WiFi + Cell
        @"iPad5,4":  @9.7,

        // iPad Pro 9.7i 1 WiFi
        @"iPad6,3":  @9.7,
        // iPad Pro 9.7i 1 WiFi + Cell
        @"iPad6,4":  @9.7,

        // iPad Pro 12.9i 1 WiFi
        @"iPad6,7":  @12.9,
        // iPad Pro 12.9i 1 WiFi + Cell
        @"iPad6,8":  @12.9,

        // iPad 5 WiFi
        @"iPad6,11": @9.7,
        // iPad 5 WiFi + Cell
        @"iPad6,12": @9.7,

        // iPad Pro 12.9i 2 WiFi
        @"iPad7,1":  @12.9,
        // iPad Pro 12.9i 2 WiFi + Cell
        @"iPad7,2":  @12.9,
        // iPad Pro 10.5i 2 WiFi
        @"iPad7,3":  @10.5,
        // iPad Pro 10.5i 2 WiFi + Cell
        @"iPad7,4":  @10.5,

        // iPad 6 WiFi
        @"iPad7,5":  @9.7,
        // iPad 6 WiFi + Cell
        @"iPad7,6":  @9.7,

        // iPad 7 WiFi
        @"iPad7,11": @10.2,
        // iPad 7 WiFi + Cell
        @"iPad7,12": @10.2,

        // iPad Pro 11i 1 WiFi
        @"iPad8,1":  @11.0,
        @"iPad8,2":  @11.0,

        // iPad Pro 11i 1 WiFi + Cell
        @"iPad8,3":  @11.0,
        @"iPad8,4":  @11.0,

        // iPad Pro 12.9i 3 WiFi
        @"iPad8,5":  @12.9,
        @"iPad8,6":  @12.9,

        // iPad Pro 12.9i 3 WiFi + Cell
        @"iPad8,7":  @12.9,
        @"iPad8,8":  @12.9,

        // iPad Pro 11i 2 WiFi
        @"iPad8,9":  @11.0,
        // iPad Pro 11i 2 WiFi + Cell
        @"iPad8,10":  @11.0,

        // iPad Pro 12.9i 4 WiFi
        @"iPad8,11":  @12.9,
        // iPad Pro 12.9i 4 WiFi + Cell
        @"iPad8,12":  @12.9,

        // iPad mini 5 WiFi
        @"iPad11,1": @7.9,
        // iPad mini 5 WiFi + Cell
        @"iPad11,2": @7.9,

        // iPad Air 3 Wi-Fi
        @"iPad11,3": @10.5,
        // iPad Air 3 Wi-Fi + Cell
        @"iPad11,4": @10.5,

        // iPad 8 WiFi
        @"iPad11,6": @10.2,
        // iPad 8 WiFi + Cell
        @"iPad11,7": @10.2,

        // iPad 9 WiFi
        @"iPad12,1": @10.2,
        // iPad 9 WiFi + Cell
        @"iPad12,2": @10.2,

        // iPad Air 4 WiFi
        @"iPad13,1": @10.9,
        // iPad Air 4 WiFi + Cell
        @"iPad13,2": @10.9,

        // iPad Pro 11i 3 WiFi
        @"iPad13,4":  @11.0,
        @"iPad13,5":  @11.0,
        // iPad Pro 11i 4 WiFi + Cell
        @"iPad13,6":  @11.0,
        @"iPad13,7":  @11.0,

        // iPad Pro 11i 4 WiFi
        @"iPad14,3":  @11.0,
        // iPad Pro 11i 4 WiFi + Cell
        @"iPad14,4":  @11.0,

        // iPad Pro 12.9i 6 WiFi
        @"iPad14,5":  @12.9,
        // iPad Pro 12.9i 6 WiFi + Cell
        @"iPad14,6":  @12.9,

        // iPad Pro 12.9i 4 WiFi
        @"iPad13,8":  @12.9,
        @"iPad13,9":  @12.9,
        // iPad Pro 12.9i 4 WiFi + Cell
        @"iPad13,10":  @12.9,
        @"iPad13,11":  @12.9,

        // iPad Air 5 WiFi
        @"iPad13,16": @10.9,
        // iPad Air 5 WiFi + Cell
        @"iPad13,17": @10.9,

        // iPad 10 WiFi
        @"iPad13,18": @10.9,
        // iPad 10 WiFi + Cell
        @"iPad13,19": @10.9,

        // iPad mini 6 WiFi
        @"iPad14,1": @8.3,
        // iPad mini 6 WiFi + Cell
        @"iPad14,2": @8.3,

        // iPad Air 11i M2 WiFi
        @"iPad14,8": @11.0,
        // iPad Air 11i M2 WiFi + Cell
        @"iPad14,9": @11.0,

        // iPad Air 13i M2 WiFi
        @"iPad14,10": @13.0,
        // iPad Air 13i M2 WiFi + Cell
        @"iPad14,11": @13.0,

        // iPad Air 11i M3 WiFi
        @"iPad15,3": @11.0,
        // iPad Air 11i M3 WiFi + Cell
        @"iPad15,4": @11.0,

        // iPad Air 13i M3 WiFi
        @"iPad15,5": @13.0,
        // iPad Air 13i M3 WiFi + Cell
        @"iPad15,6": @13.0,


        // iPad A16 WiFi
        @"iPad15,7": @10.9,
        // iPad A16 WiFi + Cell
        @"iPad15,8": @10.9,

        // iPad mini A17 Pro WiFi
        @"iPad16,1": @8.3,
        // iPad mini A17 Pro WiFi + Cell
        @"iPad16,2": @8.3,

        // iPad Pro 11i M4 WiFi
        @"iPad16,3":  @11.0,
        // iPad Pro 11i M4 WiFi + Cell
        @"iPad16,4":  @11.0,

        // iPad Pro 13i M4 WiFi
        @"iPad16,5":  @13.0,
        // iPad Pro 13i M4 WiFi + Cell
        @"iPad16,6":  @13.0,

        // iPad Air 11i M4 WiFi
        @"iPad16,8": @11.0,
        // iPad Air 11i M4 WiFi + Cell
        @"iPad16,9": @11.0,

        // iPad Air 13i M4 WiFi
        @"iPad16,10": @13.0,
        // iPad Air 13i M4 WiFi + Cell
        @"iPad16,11": @13.0,

        // iPad Pro 11i M5 WiFi
        @"iPad17,1":  @11.0,
        // iPad Pro 11i M5 WiFi + Cell
        @"iPad17,2":  @11.0,
        // iPad Pro 13i M5 WiFi
        @"iPad17,3":  @13.0,
        // iPad Pro 13i M5 WiFi + Cell
        @"iPad17,4":  @13.0,

        // iPod Touch 1
        @"iPod1,1": @3.5,
        // iPod Touch 2
        @"iPod2,1": @3.5,
        // iPod Touch 3
        @"iPod3,1": @3.5,
        // iPod Touch 4
        @"iPod4,1": @3.5,
        // iPod Touch 5
        @"iPod5,1": @4.0,
        // iPod Touch 6
        @"iPod7,1": @4.0,
        // iPod Touch 7 (last)
        @"iPod9,1": @4.0,
    };

    uname(&systemInfo);
    deviceName = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
    foundDiag = devices[deviceName];

    if(foundDiag)
        diag = (int)[foundDiag integerValue];

    *ow = w;
    *oh = h;

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
    devices =
    @{
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

int ios_get_overscan_pix_size(void)
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
    devices =
    @{
        // X
        @"iPhone10,3": @50,
        @"iPhone10,6": @50,
        // XR
        @"iPhone11,8": @50,
        // XS
        @"iPhone11,2": @50,
        // XS Max
        @"iPhone11,4": @50,
        @"iPhone11,6": @50,
        // 11
        @"iPhone12,1": @50,
        // 11 Pro
        @"iPhone12,3": @50,
        // 11 Pro Max
        @"iPhone12,5": @50,
        // 12 mini
        @"iPhone13,1": @50,
        // 12
        @"iPhone13,2": @50,
        // 12 Pro
        @"iPhone13,3": @50,
        // 12 Pro Max
        @"iPhone13,4": @50,
        // 13 mini
        @"iPhone14,4": @50,
        // 13
        @"iPhone14,5": @50,
        // 13 Pro
        @"iPhone14,2": @50,
        // 13 Pro Max
        @"iPhone14,3": @50,
        // 14
        @"iPhone14,7": @50,
        // 14 Plus
        @"iPhone14,8": @50,

        // These devices now have different cut off

        // 14 Pro
        @"iPhone15,2": @50,
        // 14 Pro Max
        @"iPhone15,3": @50,
        // 15
        @"iPhone15,4": @50,
        // 15 Plus
        @"iPhone15,5": @50,
        // 15 Pro
        @"iPhone16,1": @50,
        // 15 Pro Max
        @"iPhone16,2": @50,
        // 16 Pro
        @"iPhone17,1": @50,
        // 16 Pro Max
        @"iPhone17,2": @50,
        // 16
        @"iPhone17,3": @50,
        // 16 Plus
        @"iPhone17,4": @50,
        // 17
        @"iPhone18,3": @50,
        // Air
        @"iPhone18,4": @50,
        // 17 Pro
        @"iPhone18,1": @50,
        // 17 Pro Max
        @"iPhone18,2": @50,
        // 17e
        @"iPhone18,5": @50,
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
__strong static CHHapticEngine *s_hapticsEngine = nil;

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
        {
            pLogWarning("iOS: Failed to initialise the Haptics Engine: %s", [error.localizedDescription UTF8String]);
            return -1;
        }
        
        s_hapticsSupported = 0;
        ++s_hapticsCounter;
        
        [s_hapticsEngine setResetHandler:^
        {
            pLogInfo("iOS: Haptics Engine RESET!");

            // Try restarting the engine again.
            NSError* startupError;
            [s_hapticsEngine startAndReturnError:&startupError];
            
            if(startupError)
                pLogWarning("iOS: Haptics Engine couldn't restart!: %s", [startupError.localizedDescription UTF8String]);
            
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
                pLogWarning("iOS: Haptics Engine stopped: Audio Session Interrupt");
                break;
            }
            case CHHapticEngineStoppedReasonApplicationSuspended:
            {
                pLogWarning("iOS: Haptics Engine stopped: Application Suspended");
                break;
            }
            
            case CHHapticEngineStoppedReasonIdleTimeout:
            {
                pLogWarning("iOS: Haptics Engine stopped: Idle Timeout");
                break;
            }
            
            case CHHapticEngineStoppedReasonSystemError:
            {
                pLogWarning("iOS: Haptics Engine stopped: System Error");
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

    return 0;
}

void ios_trigger_vibrator(float strenght, int ms)
{
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
    if(@available(iOS 13.0, *))
    {
        if(ios_trigger_vibrator_taps(strenght, ms) == 0)
            return;
    }
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
        if(s_hapticsSupported == 0)
        {
            [s_hapticsEngine startWithCompletionHandler:^(NSError * _Nullable e_error)
            {
                NSError* error;

                if(e_error)
                {
                    pLogWarning("iOS: Failed to start the Haptics Engine: %s", [error.localizedDescription UTF8String]);
                    return;
                }

                CHHapticEventParameter *strengthParameter = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticIntensity value:strenght];
                CHHapticEventParameter *sharpnessParameter = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticSharpness value:1.0];

                CHHapticEvent *event = [[CHHapticEvent alloc] initWithEventType:CHHapticEventTypeHapticContinuous parameters:@[strengthParameter, sharpnessParameter] relativeTime:0 duration:(ms / 1000.0f)];
                CHHapticPattern *patten = [[CHHapticPattern alloc] initWithEvents:@[event] parameterCurves:@[] error:&error];

                id<CHHapticPatternPlayer> player = [s_hapticsEngine createPlayerWithPattern:patten error:&error];

                if(error)
                {
                    pLogWarning("iOS: Failed to create the Haptics Player: %s", [error.localizedDescription UTF8String]);
                    return;
                }

                [player startAtTime:0 error:&error];

                if(error)
                {
                    pLogWarning("iOS: Failed to start the Haptics Player: %s", [error.localizedDescription UTF8String]);
                    [player release];
                    return;
                }
                
                [s_hapticsEngine notifyWhenPlayersFinished:^CHHapticEngineFinishedAction(NSError * _Nullable error)
                {
                    [s_hapticsEngine stopWithCompletionHandler:nil];
                    return CHHapticEngineFinishedActionStopEngine;
                }];
            }];

            return 0;
        }
    }
#else
    (void)strenght;
#endif

    /* Fallback for devices without haptics, and for iOS older than 13! */
    if(ms >= 450)
        AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
    else if(ms >= 12)
        AudioServicesPlaySystemSound(1520);
    else if(ms >= 6)
        AudioServicesPlaySystemSound(1519);
    else
        return -1;

    return 0;
}
