#ifndef QDPRPIXMAP_H
#define QDPRPIXMAP_H

#include <QPixmap>
#include <QApplication>
#include <QPen>
#include <QPalette>
#include <QWidget>
#include <QPainter>

class QDPRPixmap : public QPixmap
{
public:
    QDPRPixmap() : QPixmap() {
        setDevicePixelRatio(qApp->devicePixelRatio());
    }
    QDPRPixmap(const QString& src) : QPixmap(src) {
        setDevicePixelRatio(qApp->devicePixelRatio());
    }
    QDPRPixmap(const QSize& size) : QPixmap(size * qApp->devicePixelRatio())
    {
        setDevicePixelRatio(qApp->devicePixelRatio());
    }
    QDPRPixmap(const QSize& size, const QPixmap& src, const Qt::AspectRatioMode a = Qt::KeepAspectRatio) : QPixmap(src.scaled(
              size * qApp->devicePixelRatio(),
              a,
              Qt::SmoothTransformation
              )) {
        setDevicePixelRatio(qApp->devicePixelRatio());
    }
    /*
    QDPRPixmap(const QSize& size, const QPixmap& src, const Qt::AspectRatioMode a = Qt::IgnoreAspectRatio) : QPixmap(size * qApp->devicePixelRatio()) {
        QPixmap::setDevicePixelRatio(qApp->devicePixelRatio());
        QPixmap::fill(Qt::transparent);
        QPainter p(this);
        p.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing);
        QRect r(realRect());
        r.setSize(src.size().scaled(realSize(),a));
        r.moveCenter(realRect().center());
        p.drawPixmap(r,src,src.rect());
        p.end();
    }
*/
    QDPRPixmap(const QPixmap& src, const Qt::AspectRatioMode a = Qt::IgnoreAspectRatio) : QDPRPixmap(src.size() / src.devicePixelRatio(), src, a) {}
    QDPRPixmap(const QSize& size, const QString& src, const Qt::AspectRatioMode a = Qt::IgnoreAspectRatio) : QPixmap(QPixmap(src).scaled(
              size * qApp->devicePixelRatio(),
              a,
              Qt::SmoothTransformation
              )) {
        setDevicePixelRatio(qApp->devicePixelRatio());
    }

    QDPRPixmap(const QSize& size, const QImage& src, const Qt::AspectRatioMode a = Qt::KeepAspectRatio) : QPixmap(QPixmap::fromImage(src).scaled(
              size * qApp->devicePixelRatio(),
              a,
              Qt::SmoothTransformation
              )) {
        setDevicePixelRatio(qApp->devicePixelRatio());
    }
    static QSize realSize(QSize s) {
        return QSize(s / qApp->devicePixelRatio());
    }
    static QSize realSize(const QPixmap& p) {
        return realSize(p.size());
    }
    QSize realSize() {
        return realSize(QPixmap::size());
    }
    static QRect realRect(QRect r) {
        return QRect(r.topLeft() / qApp->devicePixelRatio(), r.bottomRight() / qApp->devicePixelRatio());
    }
    static QRect realRect(const QPixmap& p) {
        return realRect(p.rect());
    }
    QRect realRect() {
        return realRect(QPixmap::rect());
    }
    static void setWidgetBackground(QWidget* w, const QString& img, QPalette::ColorRole r = QPalette::Window) {
        QPalette pal = w->palette();
        pal.setBrush(r,QBrush(QDPRPixmap(img)));
        w->setPalette(pal);
    }
    QPixmap& shadowedPixmap() {
        if (m_ShadowedPixmap.isNull()) {
            m_ShadowedPixmap = QDPRPixmap((QPixmap::size() / qApp->devicePixelRatio()) + QSize(8,8));
            m_ShadowedPixmap.fill(Qt::transparent);
            QPainter p(&m_ShadowedPixmap);
            p.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing);
            for (int i = 0; i < 8; i++) p.drawPixmap(i + 1, i + 1, shadow());
            p.drawPixmap(0,0,*this);
            p.end();
        }
        return m_ShadowedPixmap;
    }
    QPixmap& activeShadow() {
        if (m_ActiveShadow.isNull()) {
            m_ActiveShadow = QDPRPixmap((QPixmap::size() / qApp->devicePixelRatio()) + QSize(16,16));
            m_ActiveShadow.fill(Qt::transparent);
            QPainter p(&m_ActiveShadow);
            p.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing);
            for (int i = 8; i < 15; i++) p.drawPixmap(i + 1, i + 1, shadow());
            p.end();
        }
        return m_ActiveShadow;
    }
    QPixmap& inactiveShadow() {
        if (m_InactiveShadow.isNull()) {
            m_InactiveShadow = QDPRPixmap((QPixmap::size() / qApp->devicePixelRatio()) + QSize(11,11));
            m_InactiveShadow.fill(Qt::transparent);
            QPainter p(&m_InactiveShadow);
            p.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing);
            for (int i = 8; i < 10; i++) p.drawPixmap(i + 1, i + 1, shadow());
            p.end();
        }
        return m_InactiveShadow;
    }
private:
    QPixmap m_Shadow;
    QPixmap m_ShadowedPixmap;
    QPixmap m_ActiveShadow;
    QPixmap m_InactiveShadow;
    QPixmap& shadow() {
        if (m_Shadow.isNull()) {
            m_Shadow = QDPRPixmap(QPixmap::size() / qApp->devicePixelRatio());
            m_Shadow.fill(Qt::transparent);
            QPainter p(&m_Shadow);
            p.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing);
            p.fillRect(m_Shadow.rect(), QColor(0, 0, 0, 10));
            p.end();
            m_Shadow.setMask(QPixmap::mask());
        }
        return m_Shadow;
    }
};

#endif // QDPRPIXMAP_H
