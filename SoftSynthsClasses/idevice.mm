#include "idevice.h"
#ifdef Q_OS_IOS
  #import <UIKit/UIKit.h>
  #define _WINDOW UIWindow
  #define _VIEW UIView
  #define _IMAGE UIImage
#else
  #import <AppKit/AppKit.h>
  #define _WINDOW NSWindow
  #define _VIEW NSView
  #define _IMAGE NSImage
#endif
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include <QWindow>
#include <QGuiApplication>
#include <QPixmap>

IDevice::~IDevice() {
    QMutexLocker locker(&mutex);
    qDebug() << "~IDevice";
    if (m_Form != nullptr)
    {
        m_Form->hide();
        m_Form->stopTimer();
        m_Form->deleteLater();
        m_Form = nullptr;
    }
    qDeleteAll(m_Jacks);
    qDeleteAll(m_Parameters);
    qDeleteAll(m_ParameterGroups);
    if (m_FileParameter) delete m_FileParameter;
}

void mac_raise(QWidget* w)
{
    _WINDOW* nsw = [(__bridge _VIEW*)reinterpret_cast<void*>(w->winId()) window];
#ifdef Q_OS_IOS
    [nsw makeKeyAndVisible];
#else
    [nsw makeKeyAndOrderFront:nil];
#endif
}

void fixMaximizeButton(QWidget *w, bool resizable)
{
#ifdef Q_OS_IOS
    Q_UNUSED(w)
    Q_UNUSED(resizable)
#else
    _WINDOW* nsw = [(__bridge _VIEW*)reinterpret_cast<void*>(w->winId()) window];
    NSWindowStyleMask m = [nsw styleMask];
    if(resizable)
        [nsw setStyleMask:m | NSWindowStyleMaskResizable];
    else
        [nsw setStyleMask:m & ~NSWindowStyleMaskResizable];
#endif
}

int nativeAlert(QWidget *parent, QString messageText, QString informativeText, QStringList buttons) {
#ifdef Q_OS_IOS
    __block auto loop = std::make_unique<QEventLoop>();
    __block int result = 0;  // Variabeln måste vara __block för att kunna ändras i Objective-C blocket

    UIAlertController *alert = [UIAlertController
                                 alertControllerWithTitle:messageText.toNSString()
                                 message:informativeText.toNSString()
                                 preferredStyle:UIAlertControllerStyleAlert];

    for (int i = 0; i < buttons.size(); i++) {
        UIAlertAction* action = [UIAlertAction actionWithTitle:buttons[i].toNSString()
                                                         style:UIAlertActionStyleDefault
                                                       handler:^(UIAlertAction * _Nonnull /*action*/) {
            result = 1000 + i;
            loop->quit();  // Avsluta event-loopen när användaren gjort sitt val
        }];
        [alert addAction:action];
    }

    [[[(__bridge _VIEW*)reinterpret_cast<void*>(parent->winId()) window] rootViewController] presentViewController:alert animated:YES completion:nil];

    loop->exec();  // Vänta tills användaren tryckt på en knapp
    return result;
#else
    NSAlert *alert = [[NSAlert alloc] init];

    for (const auto &button : buttons) {
        [alert addButtonWithTitle:button.toNSString()];
    }

    [alert setMessageText:messageText.toNSString()];
    [alert setInformativeText:informativeText.toNSString()];
    [alert setAlertStyle:NSAlertStyleInformational];
    CGImageRef cgImg = parent->windowIcon().pixmap(512).toImage().toCGImage();
    if (cgImg) {
      _IMAGE* img = [[_IMAGE alloc] initWithCGImage:cgImg size:NSZeroSize];
      [alert setIcon:img];
      CGImageRelease(cgImg);
    }
    int i = [alert runModal];
    //return [alert beginSheetModalForWindow:parent->window()];
    return i;
#endif
}

void nativeMessage(QWidget *parent, QString messageText, QString informativeText) {
#ifdef Q_OS_IOS
    UIAlertController *alert = [UIAlertController
                                 alertControllerWithTitle:messageText.toNSString()
                                 message:informativeText.toNSString()
                                 preferredStyle:UIAlertControllerStyleAlert];



        UIAlertAction* a = [UIAlertAction actionWithTitle:@"Ok" style:UIAlertActionStyleDefault handler:^(UIAlertAction * /*action*/) {
                                   //Handle no, thanks button
                               }];
        [alert addAction:a];
/*
    CGImageRef cgImg = parent->windowIcon().pixmap(512).toImage().toCGImage();
    if (cgImg) {
      _IMAGE* img = [[_IMAGE alloc] initWithCGImage:cgImg size:NSZeroSize];
      [alert setIcon:img];
      CGImageRelease(cgImg);
    }
    [alert runModal];
*/
    [[[(__bridge _VIEW*)reinterpret_cast<void*>(parent->winId()) window] rootViewController] presentModalViewController:alert animated:YES];
#else

    NSAlert *alert = [[NSAlert alloc] init];

    [alert addButtonWithTitle:QString("Ok").toNSString()];

    [alert setMessageText:messageText.toNSString()];
    [alert setInformativeText:informativeText.toNSString()];
    [alert setAlertStyle:NSAlertStyleInformational];
    CGImageRef cgImg = parent->windowIcon().pixmap(512).toImage().toCGImage();
    if (cgImg) {
      _IMAGE* img = [[_IMAGE alloc] initWithCGImage:cgImg size:NSZeroSize];
      [alert setIcon:img];
      CGImageRelease(cgImg);
    }
    [alert runModal];
#endif
}

WId windowNumberOfWidget(QWidget* w) {
    void* view = (void*)w->winId();
#ifdef Q_OS_IOS
    return [[(__bridge _VIEW*)view window] windowLevel];
#else
    return [[(__bridge _VIEW*)view window] windowNumber];
#endif
}

void setWindowInFrontOf(QWidget* w, QWidget* w1) {
    void* view = (void*)w->winId();
#ifdef Q_OS_IOS
    void* view1 = (void*)w1->winId();
    UIView* superview = [(__bridge _VIEW*)view1 superview];  // Hämta parent-vyn
    if (superview) {
        [superview bringSubviewToFront:(__bridge _VIEW*)view];  // Flytta fram vyn i iOS
    }
    //[[(__bridge _VIEW*)view window] setWindowLevel:(NSInteger)windowNumberOfWidget(w1) + 1];
#else
    [[(__bridge _VIEW*)view window] orderWindow:NSWindowAbove relativeTo:(NSInteger)windowNumberOfWidget(w1)];
#endif
}

bool isWindowPartlyVisible(QWidget* w) {
    void* view = (void*)w->winId();
#ifdef Q_OS_IOS
    if ([(__bridge _VIEW*)view window] == nil) return false;
    UIView* currentView = (__bridge UIView*)view;
    while (UIView* superview = currentView.superview) {
        if (!CGRectIntersectsRect(superview.bounds,currentView.frame)) return false;
        if (currentView.isHidden) return false;
        currentView = superview;
    }
    return true;
#else
    return ([[(__bridge _VIEW*)view window] occlusionState] & NSWindowOcclusionStateVisible);
#endif
}
