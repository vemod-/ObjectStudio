#ifndef MACSTRINGS_H
#define MACSTRINGS_H

#include <QString>

const QString qt_mac_MacRomanToQString (const char* source);

const QString qt_mac_NSStringToQString(const void *source);

const QString qt_mac_CFStringToQString(const void* source);

#endif // MACSTRINGS_H
