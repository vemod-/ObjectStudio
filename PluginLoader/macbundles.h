#ifndef MACBUNDLES_H
#define MACBUNDLES_H

//#include <QtCore>
#include <QString>
#include <QDebug>
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>

CFURLRef pathToCFURLRef(const QString& path)
{
    CFStringRef bundlePath = path.toCFString(); //CFStringCreateWithCString(kCFAllocatorDefault, path.toUtf8().constData(), kCFStringEncodingUTF8 );
    CFURLRef bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, bundlePath, kCFURLPOSIXPathStyle, true);
    CFRelease(bundlePath);
    return bundleURL;
}

CFBundleRef pathToCFBundleRef(const QString& path)
{
    CFURLRef bundleURL=pathToCFURLRef(path);
    CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, bundleURL);
    CFRelease(bundleURL);
    return bundle;
}

void* functionPointerInBundle(const QString& functionName, CFBundleRef bundle)
{
    CFStringRef fncName = functionName.toCFString();//CFStringCreateWithCString(kCFAllocatorDefault, functionName.toUtf8().constData(), kCFStringEncodingUTF8 );
    void* fnc = nullptr;
    try {
        qDebug() << "Trying " + functionName << " in " << bundle;
        fnc = CFBundleGetFunctionPointerForName(bundle, fncName);
        qDebug() << fnc;
    }
    catch(...) {
        qDebug() << "Catched CFBundleGetFunctionPointerForName" << fncName;
    }
    CFRelease(fncName);
    return fnc;
}

CFArrayRef bundleArchitechtures(const QString& path)
{
    CFURLRef bundleURL=pathToCFURLRef(path);
    CFArrayRef archArrayRef = CFBundleCopyExecutableArchitecturesForURL(bundleURL);
    CFRelease(bundleURL);
    return archArrayRef;
}

bool bundleIsI386(const QString& path)
{
    BOOL isI386=false;
    CFArrayRef archArrayRef = bundleArchitechtures(path);
    if (archArrayRef) isI386 = [(__bridge NSArray*)archArrayRef containsObject:[NSNumber numberWithInt:kCFBundleExecutableArchitectureI386]];
    if (archArrayRef) CFRelease(archArrayRef);
    return isI386;
}

bool bundleIsX86_64(const QString& path)
{
    BOOL X86_64=false;
    CFArrayRef archArrayRef = bundleArchitechtures(path);
    if (archArrayRef) X86_64 = [(__bridge NSArray*)archArrayRef containsObject:[NSNumber numberWithInt:kCFBundleExecutableArchitectureX86_64]];
    if (archArrayRef) CFRelease(archArrayRef);
    return X86_64;
}

#endif // MACBUNDLES_H
