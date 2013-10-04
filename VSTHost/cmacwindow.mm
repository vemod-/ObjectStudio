#include "cmacwindow.h"
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>

CMacWindow::CMacWindow(QWidget* parent):QMacCocoaViewContainer(0,parent)
{
    cocoaWin=0;
}

CMacWindow::~CMacWindow()
{
    DestroyMacWindow();
    qDebug() << "Exit CMacWindow";
}

void CMacWindow::CreateMacWindow()
{
    QRect r(rect());
    r.translate(mapToGlobal(QPoint(0,0)));
    Rect r1;
    r1.top=r.top();
    r1.left=r.left();
    r1.right=r.right();
    r1.bottom=r.bottom();

    WindowRef carbonWindow;
    CreateNewWindow(kFloatingWindowClass,
                    kWindowStandardHandlerAttribute | kWindowCompositingAttribute |
                    kWindowOpaqueForEventsAttribute | kWindowNoTitleBarAttribute | kWindowNoShadowAttribute, &r1, &carbonWindow);

    NSWindow *mixedWindow = [[NSWindow alloc] initWithWindowRef:carbonWindow];
    NSView* cocoaView = [[NSView alloc] initWithFrame:NSMakeRect(0,0,r.width(),r.height())];
    setCocoaView(cocoaView);
    hostView=cocoaView;
    NSWindow* hostWindow = [cocoaView window];
    [hostWindow addChildWindow:mixedWindow ordered:NSWindowAbove];
    cocoaWin=mixedWindow;
    qDebug() << "MacWindow Created" << carbonWindow << mixedWindow;
}

void* CMacWindow::WindowReference()
{
    return [(NSWindow*)cocoaWin windowRef];
}

void CMacWindow::DestroyMacWindow()
{
    if (cocoaWin)
    {
        NSWindow* hostWindow = [(NSView*)hostView window];
        [hostWindow removeChildWindow:(NSWindow*)cocoaWin];
        [(NSView*)hostView release];
        if (cocoaView() != NULL)
        {
            NSView* v=(NSView*)cocoaView();
            qDebug() << [v retainCount];
            //setCocoaView(NULL);
            qDebug() << [v retainCount];
        }
        DisposeWindow((WindowRef)([(NSWindow*)cocoaWin windowRef]));
        [(NSWindow*)cocoaWin release];
        cocoaWin=0;
        qDebug() << "MacWindow Destroyed";
    }
}

void CMacWindow::Init()
{
    DestroyMacWindow();
    CreateMacWindow();
}

QRect NSRectToQRect(NSRect r)
{
    return QRect(r.origin.x,[[NSScreen mainScreen] frame].size.height - (r.origin.y + r.size.height),r.size.width,r.size.height);
}

NSRect QRectToNSRect(QRect r)
{
    return NSMakeRect(r.left(),[[NSScreen mainScreen] frame].size.height - r.bottom(),r.width(),r.height());
}

void CMacWindow::resizeEvent(QResizeEvent *e)
{
    QMacCocoaViewContainer::resizeEvent(e);
    if (cocoaWin)
    {
        QRect r(rect().translated(mapToGlobal(QPoint(0,0))));
        if (r.size() != NSRectToQRect([(NSWindow*)cocoaWin frame]).size())
        {
            //[(NSWindow*)cocoaWin setFrame:QRectToNSRect(r) display:YES];
            SizeWindow((WindowRef)([(NSWindow*)cocoaWin windowRef]),r.width(),r.height(),true);
        }
    }
}

void CMacWindow::paintEvent(QPaintEvent* e)
{
    QMacCocoaViewContainer::paintEvent(e);
    if (cocoaWin)
    {
        NSWindow* hostWindow = [(NSView*)hostView window];
        //qDebug() << "levels" << [(NSWindow*)cocoaWin level] << [hostWindow level];
        if ([(NSWindow*)cocoaWin level]==[hostWindow level])
        {
            QRect r(rect().translated(mapToGlobal(QPoint(0,0))));
            if (r.topLeft() != NSRectToQRect([(NSWindow*)cocoaWin frame]).topLeft())
            {
                //[(NSWindow*)cocoaWin setFrame:QRectToNSRect(r) display:YES];
                //[(NSWindow*)cocoaWin setFrameTopLeftPoint:NSMakePoint(r.left(),r.top())];
                MoveWindow((WindowRef)([(NSWindow*)cocoaWin windowRef]),r.left(),r.top(),true);
                //[(NSWindow*)cocoaWin setFrameOrigin:NSMakePoint(r.left(),[[NSScreen mainScreen] frame].size.height - r.bottom())];
                //[(NSWindow*)cocoaWin display];
            }
            [hostWindow setLevel:[hostWindow level]];
            [hostWindow addChildWindow:(NSWindow*)cocoaWin ordered:NSWindowAbove];
            [(NSWindow*)cocoaWin orderWindow:NSWindowAbove relativeTo:[hostWindow windowNumber]];
        }
    }
    //qDebug() << "paintevent";
}

void CMacWindow::hideEvent(QHideEvent *e)
{
    if (cocoaWin) [(NSWindow*)cocoaWin orderOut:nil];
    QMacCocoaViewContainer::hideEvent(e);
    //qDebug() << "hideevent";
}
