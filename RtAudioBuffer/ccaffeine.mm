#include "ccaffeine.h"
#if defined(Q_OS_IOS)
#import <UIKit/UIKit.h>
#endif

void CCaffeine::enable()
{
#ifdef Q_OS_MACOS
    IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                kIOPMAssertionLevelOn,
                                CFStringCreateWithCString(kCFAllocatorDefault, "Caffeine", kCFStringEncodingUTF8),
                                &assertionID);
#elif defined(Q_OS_IOS)
    [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
#endif
}

void CCaffeine::disable()
{
#ifdef Q_OS_MACOS
    if (assertionID)
        IOPMAssertionRelease(assertionID);
#elif defined(Q_OS_IOS)
    [[UIApplication sharedApplication] setIdleTimerDisabled:NO];
#endif
}
