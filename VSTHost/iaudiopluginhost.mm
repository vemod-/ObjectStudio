#include "iaudiopluginhost.h"
#include "QMacFunctions"
#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>

IAudioPlugInHost::IAudioPlugInHost(unsigned int sampleRate, unsigned int bufferSize, QWidget *parent):CMacWindow(parent)
{
    m_Samplerate=sampleRate;
    m_Buffersize=bufferSize;
    m_MIDIChannel=0;
}
QPixmap IAudioPlugInHost::Picture()
{
    QPixmap pixmap(size());
    QRect r(rect());
    r.translate(mapToGlobal(QPoint(0,0)));
    CGRect rect;
    rect.origin.x=r.left();
    rect.origin.y=r.top();
    rect.size.width=r.width();
    rect.size.height=r.height();
    CGWindowID wid;
    if (cocoaWin)
    {
        wid = (CGWindowID)([(NSWindow*)cocoaWin windowNumber]);
    }
    else
    {
        wid = (CGWindowID)([(NSWindow*)([(NSView*)cocoaView() window]) windowNumber]);
    }
    CGImageRef windowImage =  CGWindowListCreateImage(rect, kCGWindowListOptionIncludingWindow, wid, kCGWindowImageDefault );
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    pixmap = QPixmap::fromMacCGImageRef(windowImage);
#else
    pixmap = QtMacExtras::fromCGImageRef(windowImage);
#endif
    return pixmap;
}
