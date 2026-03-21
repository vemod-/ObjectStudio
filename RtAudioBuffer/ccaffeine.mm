#include "ccaffeine.h"
//#if defined(Q_OS_IOS)
//#import <UIKit/UIKit.h>
//#endif

#ifdef Q_OS_MACOS
#include <CoreFoundation/CoreFoundation.h>
#endif

CCaffeine::CCaffeine()
{
    qDebug() << "caffeine construct";
    enable();
}

CCaffeine::~CCaffeine()
{
    disable();
}

void CCaffeine::setReason(const QString &reason)
{
#ifdef Q_OS_MACOS

    m_reason = reason;

    if (assertionID == kIOPMNullAssertionID)
        return;

    CFStringRef cfReason = reason.toCFString();
    IOPMAssertionSetProperty(assertionID, CFSTR("Name"), cfReason);
    CFRelease(cfReason);

#elif defined(Q_OS_IOS)

    m_reason = reason;

#endif
}

void CCaffeine::enable()
{
#ifdef Q_OS_MACOS

    if (assertionID != kIOPMNullAssertionID)
        return;

    CFStringRef cfReason = m_reason.toCFString();

    IOPMAssertionCreateWithName(
        kIOPMAssertionTypeNoDisplaySleep,
        kIOPMAssertionLevelOn,
        cfReason,
        &assertionID);

    CFRelease(cfReason);

#elif defined(Q_OS_IOS)

    [[UIApplication sharedApplication] setIdleTimerDisabled:YES];

#endif
}

void CCaffeine::disable()
{
#ifdef Q_OS_MACOS

    if (assertionID != kIOPMNullAssertionID)
    {
        IOPMAssertionRelease(assertionID);
        assertionID = kIOPMNullAssertionID;
    }

#elif defined(Q_OS_IOS)

    [[UIApplication sharedApplication] setIdleTimerDisabled:NO];

#endif
}