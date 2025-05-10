#include "cmacwindow.h"
#ifdef Q_OS_IOS
  #import <UIKit/UIKit.h>
  #define _SIZE CGSize
  #define _RECT CGRect
  #define _VIEW UIView
  #define _WINDOW UIWindow
#else
  #import <AppKit/AppKit.h>
  #define _SIZE NSSize
  #define _RECT NSRect
  #define _VIEW NSView
  #define _WINDOW NSWindow
#endif
#include <QHBoxLayout>
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
#elif (QT_VERSION < QT_VERSION_CHECK(5,2,0))
#include "QMacFunctions"
#elif (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QtMac>
#endif
/*
@interface MyViewController : NSViewController
- (void)viewWillLayout;
- (void)preferredContentSizeDidChange:(NSViewController*)viewController;
@end
@implementation MyViewController
- (void)viewWillLayout {
    qDebug() << "ViewWillLayouyt" << self.view.frame.size.width << self.view.frame.size.height;
}
- (void)preferredContentSizeDidChange:(NSViewController*)viewController {
    qDebug() << "preferredContentSizeDidChange" << viewController.preferredContentSize.width << viewController.preferredContentSize.height;
}
@end
*/
#define NSVIEW(w) ((__bridge _VIEW*)w)

#define VOID(w) ((__bridge void*)w)

inline const QSize NSSizeToQSize(_SIZE s) {
    return QSize(s.width,s.height);
}

inline const QRectF NSRectToQRect(_RECT r) {
#ifdef Q_OS_IOS
    return QRectF(r.origin.x,r.origin.y,r.size.width,r.size.height);
#else
    return QRectF(r.origin.x,[[NSScreen mainScreen] frame].size.height - (r.origin.y + r.size.height),r.size.width,r.size.height);
#endif
}

inline _RECT QRectToNSRect(const QRectF& r) {
#ifdef Q_OS_IOS
    return CGRectMake(r.left(),r.top(),r.width(),r.height());
#else
    return NSMakeRect(r.left(),r.top(),r.width(),r.height());
#endif
}
/*
NSView* viewFromID(WId winId) {
    return NSVIEW((void*)winId);
}
*/
/*
@interface MyObserver: NSObject {}
- (void)frameChanged:(NSNotification*) notification;
@end

@implementation MyObserver
- (void)frameChanged:(NSNotification*) notification {
    qDebug() << ">>>>>>>>>>frameChanged %@" << notification;
}
@end
*/
CMacWindow::CMacWindow(QWidget* parent):QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    setVisible(false);
    connect(&sizeTimer,SIGNAL(timeout()),this,SLOT(checkGeometry()));
    QHBoxLayout* l = new QHBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    setLayout(l);
    w = new QWidget(this);
    w->setAttribute(Qt::WA_NativeWindow,true);
    l->addWidget(w);
}

CMacWindow::~CMacWindow()
{
    //destroyMacWindow();
    qDebug() << "~MacWindow";
    sizeTimer.stop();
    setVisible(false);
    layout()->removeWidget(w);
    delete w;
    qDebug() << "Exit CMacWindow";
}

void CMacWindow::createMacWindow(void* host)
{
    if (!host) return;
    _VIEW* cocoaView = NSVIEW(host);// : [[NSView alloc] initWithFrame:NSZeroRect];
#ifdef Q_OS_IOS
    [cocoaView setAutoresizingMask:0];
#else
    [cocoaView setAutoresizingMask:NSViewNotSizable];
#endif

    //[cocoaView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

    //[cocoaView setNeedsLayout:false];
    //[cocoaView setTranslatesAutoresizingMaskIntoConstraints:NO];
    qDebug() << "1" << cocoaView.frame.size.width << cocoaView.frame.size.height;
    QSize s = QSize(cocoaView.frame.size.width, cocoaView.frame.size.height);
/*
    const WId id = WId(cocoaView);
    //qDebug() << "2" << cocoaView.bounds.size.width << cocoaView.bounds.size.height;
    QWindow* wi = QWindow::fromWinId(id);
    wi->setGeometry(QRect(QPoint(0,0),s));
    w = new QWidget(this);
    w->setAttribute(Qt::WA_NativeWindow);
    w->winId();
    wi->setParent(w->windowHandle());
    //qDebug() << "3" << cocoaView.bounds.size.width << cocoaView.bounds.size.height;
    //wi->setGeometry(NSRectToQRect(r).toRect());
    //qDebug() << "4" << cocoaView.bounds.size.width << cocoaView.bounds.size.height;
    //w = QWidget::createWindowContainer(wi,this);
    //w->setAttribute(Qt::WA_NativeWindow,true);
    //NSWindow* win = (__bridge NSWindow*)((void*)wi->winId());
    //[win setContentView:cocoaView];
*/
/*
    w = QWidget::createWindowContainer(QWindow::fromWinId(WId(cocoaView)));
    w->setAttribute(Qt::WA_NativeWindow,true);
    //w->winId();
*/
/*
    w = new QWidget(this);
    w->setAttribute(Qt::WA_NativeWindow);
    w->createWinId();
    typedef void (*SetWindowContentViewFunction)(QPlatformWindow *window, void *nsview);
    reinterpret_cast<SetWindowContentViewFunction>(resolvePlatformFunction("setwindowcontentview"))(w->windowHandle()->handle(),(__bridge void*)cocoaView);
*/
    //w = this;//new QWidget(this);
    //w->setAttribute(Qt::WA_NativeWindow);
    //w->createWinId();
    //l->addWidget(w);
    //w->setVisible(true);
    //NSView* super = NSVIEW((void*)w->winId());
    //[super addSubview:cocoaView];
    //NSArray<__kindof NSView*>* subs = [supersuper subviews];
    //for (uint i = 0; i < subs.count; i++) [subs[i] removeFromSuperview];
    //[supersuper replaceSubview:firstsub with:cocoaView];
    //QWidget* w = new QWidget(this);
    //w->setAttribute(Qt::WA_NativeWindow,true);
    //layout()->addWidget(w);
    _VIEW* super = NSVIEW(superId());
    [super addSubview:cocoaView];

    qDebug() << [super subviews];

    //MyViewController* controller = [[MyViewController alloc] init];
    //[controller setView:cocoaView];
/*
    [super setTranslatesAutoresizingMaskIntoConstraints:NO];

    [super setPostsFrameChangedNotifications:YES];
    MyObserver *ob1 = [[MyObserver alloc] init];
    superViewObserver = (__bridge void*)ob1;
    [[NSNotificationCenter defaultCenter] addObserver:ob1 selector:@selector(frameChanged:) name:NSViewFrameDidChangeNotification object:super];
*/
/*
    [cocoaView setPostsFrameChangedNotifications:YES];
    MyObserver *ob = [[MyObserver alloc] init];
    cocoaViewObserver = (__bridge void*)ob;
    [[NSNotificationCenter defaultCenter] addObserver:ob selector:@selector(frameChanged:) name:NSViewFrameDidChangeNotification object:cocoaView];
*/
/*
    NSDictionary *viewsDictionary = NSDictionaryOfVariableBindings(cocoaView);
    [super addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:[cocoaView]" options:0 metrics:0 views:viewsDictionary]];
    [super addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[cocoaView]" options:0 metrics:0 views:viewsDictionary]];
*/
    //[super setAutoresizesSubviews:YES];
    //contentView = (__bridge void*)content;
    //[view addSubview:cocoaView];
    //NSWindow* win = (__bridge NSWindow*)((void*)WId(w->windowHandle()->handle()));
    //[win setContentView:cocoaView];
    //qDebug() << win;
    //qDebug() << w->windowHandle()->handle() << w->internalWinId() << wi << wi->handle() << win << view << w->window()->windowHandle();
    //[win setContentView: cocoaView];
    //w->createWindowContainer(wi,this,Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);

    //w = (QWidget*)(new QMacCocoaViewContainer(cocoaView,this));
    setViewSize(s);
    //setWidgetSize(s);
    //qDebug() << "5" << cocoaView.bounds.size.width << cocoaView.bounds.size.height;
    //qDebug() << "6" << cocoaView.bounds.size.width << cocoaView.bounds.size.height;
    //qDebug() << "7" << cocoaView.bounds.size.width << cocoaView.bounds.size.height;
    //hostView = (__bridge void*)super;
    //qDebug() << "8" << cocoaView.bounds.size.width << cocoaView.bounds.size.height;
}

void CMacWindow::destroyMacWindow()
{
    sizeTimer.stop();
    setVisible(false);
/*
    while ([[NSVIEW(superId()) subviews] count] > 0) {
        NSView* v = [[NSVIEW(superId()) subviews] lastObject];
        [v removeFromSuperview];
        v = nil;
        //maybe explicitly deallocate the view depending
    }
*/
/*
    if (NSView* v = NSVIEW(viewId())) {
        [v removeConstraints:[v constraints]];
        [[NSNotificationCenter defaultCenter] removeObserver:v];
        [[NSNotificationCenter defaultCenter] removeObserver:NSVIEW(superId())];
    }
*/
    layout()->removeWidget(w);
    delete w;
    w=new QWidget(this);
    w->setAttribute(Qt::WA_NativeWindow,true);
    layout()->addWidget(w);
}
/*
void CMacWindow::releaseMacWindow() {
    while ([[NSVIEW(superId()) subviews] count] > 0) {
        NSView* v = [[NSVIEW(superId()) subviews] lastObject];
        [v removeFromSuperview];
        v = nil;
        //maybe explicitly deallocate the view depending
    }
}
*/
void CMacWindow::init(void* host)
{
    destroyMacWindow();
    createMacWindow(host);
    sizeTimer.start(100);
}

void* CMacWindow::viewId() const
{
    if ([[NSVIEW(superId()) subviews] count]) return VOID([[NSVIEW(superId()) subviews] firstObject]);
    return nullptr;
}

void* CMacWindow::superId() const {
    return (void*)w->winId();
}

QSize CMacWindow::sizeHint() const {
    if (_VIEW* v = NSVIEW(viewId())) return NSSizeToQSize(v.frame.size);
    return QWidget::sizeHint();
}

QSize CMacWindow::minimumSizeHint() const {
    if (_VIEW* v = NSVIEW(viewId())) return NSSizeToQSize(v.frame.size);
    return QWidget::minimumSizeHint();
}

void CMacWindow::setViewSize(QSize s) {
    //qDebug() << "CMacWindow setviewsize" << s;
#ifdef Q_OS_IOS
    if (_VIEW* v = NSVIEW(viewId())) [v setFrame:CGRectMake(0,0,s.width(),s.height())];
#else
    if (_VIEW* v = NSVIEW(viewId())) [v setFrame:NSMakeRect(0,0,s.width(),s.height())];
#endif
}

WId CMacWindow::parentWindowNumber() const
{
    //qDebug() << "CMacWindow contentwinid";
#ifdef Q_OS_IOS
    if (_VIEW* v = NSVIEW(viewId())) if (_WINDOW* w = [v window]) return [w windowLevel];
#else
    if (_VIEW* v = NSVIEW(viewId())) if (_WINDOW* w = [v window]) return [w windowNumber];
#endif
    return WId();
}

const QRect CMacWindow::geometryOnParentWindow() const
{
    int titleBarHeight = 0;
#ifndef Q_OS_IOS
    if (_VIEW* v = NSVIEW(viewId())) {
        _WINDOW* win = [v window];
        titleBarHeight = win.frame.size.height - win.contentLayoutRect.size.height;
    }
#endif
    return geometry().translated(0,titleBarHeight);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
CGBitmapInfo CGBitmapInfoForQImage(const QImage &image)
{
    CGBitmapInfo bitmapInfo = kCGImageAlphaNone;

    switch (image.format()) {
        case QImage::Format_ARGB32:
            bitmapInfo = kCGImageAlphaFirst | kCGBitmapByteOrder32Host;
            break;
        case QImage::Format_RGB32:
            bitmapInfo = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Host;
            break;
        case QImage::Format_RGBA8888_Premultiplied:
            bitmapInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;
            break;
        case QImage::Format_RGBA8888:
            bitmapInfo = kCGImageAlphaLast | kCGBitmapByteOrder32Big;
            break;
        case QImage::Format_RGBX8888:
            bitmapInfo = kCGImageAlphaNoneSkipLast | kCGBitmapByteOrder32Big;
            break;
        case QImage::Format_ARGB32_Premultiplied:
            bitmapInfo = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host;
            break;
        default:
            break;
    }

    return bitmapInfo;
}

QImage CGImageToQImage(CGImageRef cgImage)
{
    const size_t width = CGImageGetWidth(cgImage);
    const size_t height = CGImageGetHeight(cgImage);
    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    CGContextRef context = CGBitmapContextCreate((void *)image.bits(), image.width(), image.height(), 8,
                                                 image.bytesPerLine(), colorSpace, CGBitmapInfoForQImage(image));

    // Scale the context so that painting happens in device-independent pixels
    const qreal devicePixelRatio = image.devicePixelRatio();
    CGContextScaleCTM(context, devicePixelRatio, devicePixelRatio);

    CGRect rect = CGRectMake(0, 0, width, height);
    CGContextDrawImage(context, rect, cgImage);

    CFRelease(colorSpace);
    CFRelease(context);

    return image;
}
#endif

#ifdef Q_OS_IOS
QPixmap captureView(UIView* view) {
    if (!view || !view.window || view.hidden || view.alpha == 0) return QPixmap();

    if (![NSThread isMainThread]) {
        __block QPixmap pixmap;
        dispatch_sync(dispatch_get_main_queue(), ^{
            pixmap = captureView(view);
        });
        return pixmap;
    }

    UIGraphicsBeginImageContextWithOptions(view.bounds.size, NO, [UIScreen mainScreen].scale);
    [view drawViewHierarchyInRect:view.bounds afterScreenUpdates:NO];
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    if (!image.CGImage) return QPixmap();
    qDebug() << image.CGImage;
    return QPixmap::fromImage(CGImageToQImage(image.CGImage));
}
/*
QPixmap captureWindow(UIWindow *window) {
    if (!window) return QPixmap();

    if (![NSThread isMainThread]) {
        __block QPixmap pixmap;
        dispatch_sync(dispatch_get_main_queue(), ^{
            pixmap = captureWindow(window);
        });
        return pixmap;
    }

    UIGraphicsBeginImageContextWithOptions(window.bounds.size, NO, [UIScreen mainScreen].scale);
    [window drawViewHierarchyInRect:window.bounds afterScreenUpdates:YES];
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();

    if (!image.CGImage) return QPixmap();
    return QPixmap::fromImage(CGImageToQImage(image.CGImage));
}
*/
#endif

QPixmap CMacWindow::grab(const QRect &rectangle)
{
    if (!viewId()) return QPixmap();
    const double factor = devicePixelRatioF();
    QRect copyRect = rectangle;
#ifdef Q_OS_IOS
    _VIEW* v = NSVIEW(viewId());
    if (!v) return QPixmap();
    QPixmap pm = captureView(v);
    if (copyRect == QRect(QPoint(0, 0), QSize(-1, -1))) {
        copyRect = QRect(QPoint(0,0),NSSizeToQSize(v.frame.size));
    }
#else
    if (copyRect == QRect(QPoint(0, 0), QSize(-1, -1))) {
        copyRect = rect();
    }
    CGImageRef windowImage =  CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow, parentWindowNumber(), kCGWindowImageBoundsIgnoreFraming );
    if (!windowImage) return QPixmap();
    qDebug() << windowImage;
    #if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
        QPixmap pixmap = QPixmap::fromMacCGImageRef(windowImage);
    #elif (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
        QPixmap pixmap = QtMacExtras::fromCGImageRef(windowImage);
    #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QPixmap pm = QtMac::fromCGImageRef(windowImage);
    #else
        QPixmap pm = QPixmap::fromImage(CGImageToQImage(windowImage));
    #endif
    CGImageRelease(windowImage);
#endif
#ifdef Q_OS_IOS
    QRect r(QPoint(0,0),NSSizeToQSize(v.frame.size)*factor);
#else
    QRect r(geometryOnParentWindow().topLeft()*factor,size()*factor);
#endif
    QPixmap pixmap = pm.copy(r).scaled(size(),Qt::KeepAspectRatio,Qt::SmoothTransformation).copy(copyRect);
    return pixmap;
}

void CMacWindow::checkGeometry() {
    if (isVisible()) {
        //qDebug() << "CheckGeometry";
        if (size() != sizeHint()) {
            updateGeometry();
#ifdef Q_OS_IOS
            if (_VIEW* v = NSVIEW(viewId())) v.frame.origin = CGPointMake(0,0);
#else
            if (_VIEW* v = NSVIEW(viewId())) [v setFrameOrigin:CGPointMake(0,0)];
#endif
        }
    }
}
/*
bool CMacWindow::event(QEvent* e) {
    qDebug() << e->type();
    return QWidget::event(e);
}
*/
