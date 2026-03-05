#include "cvideodesigner.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QtOpenGLWidgets/QOpenGLWidget>

CVideoItem::CVideoItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemUsesExtendedStyleOption);
    setCacheMode(QGraphicsItem::NoCache);
    setOpacity(1);

    m_player = new QMediaPlayer(this);
    m_player->setAudioOutput(nullptr);   // aldrig ljud
    m_sink = new QVideoSink(this);
    m_player->setVideoSink(m_sink);
    connect(m_sink, &QVideoSink::videoFrameChanged, this, &CVideoItem::onVideoFrameChanged);

    grabGesture(Qt::PinchGesture);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptTouchEvents(true);
}

CVideoItem::~CVideoItem() {
    m_player->stop();
    m_currentFrame = {};
    m_player->deleteLater();
    m_sink->deleteLater();
    if (m_ExportPlayer) {
        m_ExportPlayer->stop();
        m_ExportPlayer->deleteLater();
        m_ExportSink->deleteLater();
    }
}

QRectF CVideoItem::boundingRect() const
{
    return m_rect.adjusted(-m_handleSize,
                           -m_handleSize,
                           m_handleSize,
                           m_handleSize);
}

void CVideoItem::paint(QPainter* p,
                       const QStyleOptionGraphicsItem*,
                       QWidget*)
{
    if (m_ExportMode) {
        if (m_frameGeneration != m_exportGeneration){
            qDebug() << "paint empty Frame";
            return;
        }
        p->setRenderHint(QPainter::SmoothPixmapTransform, true);
        p->setRenderHint(QPainter::Antialiasing, true);
        p->drawImage(m_rect, m_currentImage, m_sourceRect);
        qDebug() << "paint frame";
        return;
    }
    if (!m_Enabled) return;
    if (!isVisible()) return;
    p->setRenderHint(QPainter::SmoothPixmapTransform, false);
    p->setRenderHint(QPainter::Antialiasing, false);
    if (!m_Playing) {
        p->drawPixmap(m_rect,thumbnail,m_sourceRect);
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(0,0,0,60));
        p->drawRect(m_rect);
    }
    else {
        if (!m_currentFrame.isValid()) return;
        const auto fmt = QVideoFrameFormat::imageFormatFromPixelFormat(m_currentFrame.pixelFormat());
        if (fmt != QImage::Format_Invalid) {
            p->drawImage(m_rect,QImage(
                                 m_currentFrame.bits(0),
                                 m_currentFrame.width(),
                                 m_currentFrame.height(),
                                 m_currentFrame.bytesPerLine(0),
                                 fmt),m_sourceRect);
        }
        else {
            p->drawImage(m_rect,m_currentFrame.toImage(),m_sourceRect);
        }
    }
    if (isSelected())
    {
        p->setPen(QPen(Qt::yellow, 2));
        p->drawRect(m_rect);
        drawHandles(p);
    }
    if (!m_Playing) {
        if (m_MD) {
            QRect r = m_sourceRect;
            if ((qApp->queryKeyboardModifiers() == Qt::NoModifier) || (m_activeHandle != NoHandle)) r = QRectF(pos() + m_rect.topLeft(),m_rect.size()).toRect();
            const QString posString = QString::number(r.left()) + "," + QString::number(r.top());
            const QString sizeString = QString::number(r.width()) + "," + QString::number(r.height());
            p->setBrush(Qt::black);
            p->setPen(QPen(Qt::white, 2));
            p->setFont(QFont("",12));
            p->drawText(m_rect.topLeft() + QPoint(10,17),posString);
            p->drawText(m_rect.bottomRight() - QPoint(60,2),sizeString);
        }
        else {
            if (!name.isEmpty()) {
                p->setBrush(Qt::black);
                p->setPen(QPen(Qt::yellow, 2));
                p->setFont(QFont("",12));
                p->drawText(m_rect.topLeft() + QPoint(10,17),name);
            }
        }
    }
}

void CVideoItem::setThumbnail(const QPixmap& pix)
{
    prepareGeometryChange();
    thumbnail = pix;
    m_frameSize = pix.size() / pix.devicePixelRatio();
    m_rect = QRect(QPoint(0,0),m_frameSize);
    m_sourceRect = m_rect;
    update();
}

void CVideoItem::onExportFrameChanged(const QVideoFrame& frame) {
    qDebug() << "onExportFrame";
    QVideoFrame f(frame);
    if (f.map(QVideoFrame::ReadOnly)) {
        auto fmt =
            QVideoFrameFormat::imageFormatFromPixelFormat(
                f.pixelFormat());

        if (fmt != QImage::Format_Invalid) {
            m_currentImage = QImage(
                                 f.bits(0),
                                 f.width(),
                                 f.height(),
                                 f.bytesPerLine(0),
                                 fmt
                                 ).copy();   // ← KRITISK
        }
        else {
            m_currentImage = f.toImage();
        }
        f.unmap();
    }
    emit frameReady();
}

void CVideoItem::onVideoFrameChanged(const QVideoFrame& frame)
{
    if (!m_Enabled) return;
    if (!m_Playing) return;
    if (!isVisible()) return;
    if (m_MD) return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_LastFrameChange < 16) return;
    m_LastFrameChange = now;
    m_currentFrame = frame;
    update();
}

void CVideoItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    if (m_Playing) return;
    m_MD = true;
    m_activeHandle = handleAt(e->pos().toPoint());
    m_pressPos = e->pos();
    m_pressSourceRect = m_sourceRect;

    if (m_activeHandle != NoHandle) {
        if (scene()) scene()->clearSelection();
        setSelected(true);
        e->accept();
        return;
    }
    update();
    QGraphicsObject::mousePressEvent(e);
}

void CVideoItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (m_Playing) return;
    QPointF delta = e->pos() - m_pressPos;
    qreal scaleX = m_pressSourceRect.width()  / m_rect.width();
    qreal scaleY = m_pressSourceRect.height() / m_rect.height();

    QPointF scaledDelta(delta.x()*scaleX, delta.y()*scaleY);

    if (m_activeHandle != NoHandle) {
        prepareGeometryChange();
        QPoint p = e->pos().toPoint();
        switch (m_activeHandle)
        {
        case TopLeft:
            m_rect.setTopLeft(p);
            break;
        case TopRight:
            m_rect.setTopRight(p);
            break;
        case BottomLeft:
            m_rect.setBottomLeft(p);
            break;
        case BottomRight:
            m_rect.setBottomRight(p);
            break;
        default:
            break;
        }
        if (e->modifiers() & Qt::ShiftModifier)
        {
            // Aspect ratio lock
            qreal ratio = (qreal)m_frameSize.width() /
                          m_frameSize.height();

            m_rect.setHeight((qreal)m_rect.width() / ratio);
        }
        update();
        return;
    }
    if (m_MD) {
        if (e->modifiers() & Qt::ControlModifier)
        {
            QRectF r = m_pressSourceRect;

            r.setBottomRight(m_pressSourceRect.bottomRight() - scaledDelta);

            if (e->modifiers() & Qt::ShiftModifier) {
                qreal ratio = (qreal)m_rect.width() / m_rect.height();
                r.setHeight((qreal)r.width() / ratio);
            }

            m_sourceRect = r.toRect();
            update();
            return;
        }
        if (e->modifiers() & Qt::ShiftModifier)
        {
            QRectF r = m_pressSourceRect;
            r.moveTopLeft( m_pressSourceRect.topLeft() - scaledDelta);

            m_sourceRect = r.toRect();
            update();
            return;
        }
    }
    QGraphicsObject::mouseMoveEvent(e);
}

void CVideoItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    if (m_Playing) return;
    m_MD = false;
    m_activeHandle = NoHandle;
    update();
    QGraphicsObject::mouseReleaseEvent(e);
}


CVideoDesigner::CVideoDesigner(QWidget* parent)
    : QGraphicsView(parent)
{
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    setBackgroundBrush(Qt::black);

    setViewport(new QOpenGLWidget());

    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    viewport()->setAttribute(Qt::WA_NoSystemBackground);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontSavePainterState);

    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setRenderHint(QPainter::Antialiasing, false);

    m_scene.setItemIndexMethod(QGraphicsScene::BspTreeIndex);
    setScene(&m_scene);

    setDragMode(QGraphicsView::RubberBandDrag);
    setTransformationAnchor(AnchorUnderMouse);
    m_scene.setSceneRect(0, 0, 640, 480); // default canvas
}

QList<CVideoItem*> CVideoDesigner::videoItems() const
{
    QList<CVideoItem*> list;
    for (auto* item : m_scene.items())
        if (auto v = dynamic_cast<CVideoItem*>(item))
            list.append(v);
    return list;
}

void CVideoDesigner::resizeEvent(QResizeEvent* e)
{
    QGraphicsView::resizeEvent(e);
}
