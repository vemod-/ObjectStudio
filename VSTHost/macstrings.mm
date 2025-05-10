#include "macstrings.h"

#import <CoreFoundation/CoreFoundation.h>
#ifndef __x86_64
//#include <Carbon/Carbon.h>
#endif

const QString qt_mac_MacRomanToQString(const char* source)
{
    CFStringRef temp = CFStringCreateWithCString(kCFAllocatorDefault, source, kCFStringEncodingMacRoman);
    QString retval=qt_mac_CFStringToQString(temp);
    CFRelease(temp);
    return retval;
}

const QString qt_mac_NSStringToQString(const void *source)
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
    if (!source) return QString();
    NSString* nsstr=(NSString*)source;
    const int length=[nsstr length];
    if (length == 0) return QString();
    QString string(length,Qt::Uninitialized);
    [nsstr getCharacters:(UniChar*)string.unicode() range:NSMakeRange(0,length)];
    return QString::fromUtf16(string.unicode(), length);
#else
    return QString::fromNSString((__bridge NSString*)source);
#endif
}

const QString qt_mac_CFStringToQString(const void* source)
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
    if (!source) return QString();
    CFStringRef str=(CFStringRef)source;
    const CFIndex length = CFStringGetLength(str);
    if (length == 0) return QString();
    QString string(length, Qt::Uninitialized);
    CFStringGetCharacters(str, CFRangeMake(0, length),(UniChar*)string.unicode());
    return string;
#else
    return QString::fromCFString(static_cast<CFStringRef>(source));
#endif
}
