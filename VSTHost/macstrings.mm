#include "macstrings.h"

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

const QString qt_mac_MacRomanToQString (const char* source)
{
    char buffer[256];
    CFStringRef temp = CFStringCreateWithCString (kCFAllocatorDefault, source, kCFStringEncodingMacRoman);
    CFStringGetCString (temp, buffer, 256, kCFStringEncodingUTF8);
    CFRelease (temp);
    return QString(buffer);
}

const QString qt_mac_NSStringToQString(const void *source)
{
    NSString* nsstr=(NSString*)source;
    NSRange range;
    range.location = 0;
    range.length = [nsstr length];
    QString result(range.length, QChar(0));

    unichar *chars = new unichar[range.length];
    [nsstr getCharacters:chars range:range];
    result = QString::fromUtf16(chars, range.length);
    delete[] chars;
    return result;
}

const QString qt_mac_CFStringToQString(const void* source)
{
    CFStringRef str=(CFStringRef)source;
    if (!str)
        return QString();

    CFIndex length = CFStringGetLength(str);
    if (length == 0)
        return QString();

    QString string(length, Qt::Uninitialized);
    CFStringGetCharacters(str, CFRangeMake(0, length), reinterpret_cast<UniChar *>
                          (const_cast<QChar *>(string.unicode())));
    return string;
}
