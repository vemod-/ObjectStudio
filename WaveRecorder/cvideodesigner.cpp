#include "cvideodesigner.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
//#include <QtOpenGLWidgets/QOpenGLWidget>

CVideoItem::CVideoItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemUsesExtendedStyleOption);
    setCacheMode(QGraphicsItem::NoCache);
    setOpacity(1);

    frameTimer.setInterval(40); // 25 fps ~60 fps

    connect(&frameTimer,&QTimer::timeout,this,[this]{
        if (!m_Playing) return;
        if (!m_Enabled) {
            update(m_rect);
            return;
        }
        if (m_AVFPlayer.isPlaying()) m_currentPlaybackImage = m_AVFPlayer.currentFrame();
        if (!m_currentPlaybackImage.isNull()) update(m_rect);
    });
    grabGesture(Qt::PinchGesture);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptTouchEvents(true);
}

CVideoItem::~CVideoItem() {
    frameTimer.stop();
    m_Playing = false;
}

QRectF CVideoItem::boundingRect() const
{
    if (m_Playing) return QRectF(m_rect).adjusted(1,1,-1,-1);
    return m_rect.adjusted(-m_handleSize, -m_handleSize, m_handleSize, m_handleSize);
}

void CVideoItem::paint(QPainter* p,
                       const QStyleOptionGraphicsItem*,
                       QWidget*)
{
    if (m_ExportMode) {
        if (m_frameGeneration != m_exportGeneration){
            return;
        }
        p->setRenderHint(QPainter::SmoothPixmapTransform, true);
        p->setRenderHint(QPainter::Antialiasing, true);
        p->setOpacity(m_opacity);
        p->drawImage(renderRect, m_exportImage, m_sourceRect);
        return;
    }
    if (m_Playing) {
        p->setOpacity(m_opacity);
        p->drawImage(m_rect,m_currentPlaybackImage,m_sourceRect);
        return;
    }
    if (!m_Enabled) return;
    //if (!isVisible()) return;
//    if (!m_Playing) {
        if (!m_stillImage.isNull()) {
            p->setRenderHint(QPainter::SmoothPixmapTransform, true);
            p->setRenderHint(QPainter::Antialiasing, true);
            p->setOpacity(m_opacity);
            p->drawImage(m_rect,m_stillImage,m_sourceRect);
        }
        else {
            p->setOpacity(0.6);
            p->drawImage(m_rect,thumbnail,m_sourceRect);
            p->setOpacity(1.0);
            //p->setPen(Qt::NoPen);
            //p->setBrush(QColor(0,0,0,60));
            //p->drawRect(m_rect);
        }
        /*
    }
    else {
        //if (m_currentPlaybackImage.isNull()) return;
        p->setOpacity(m_opacity);
        p->drawImage(m_rect,m_currentPlaybackImage,m_sourceRect);
        return;
    }
*/
    if (isSelected())
    {
        p->setBrush(Qt::NoBrush);
        p->setPen(QPen(Qt::yellow, 2));
        p->drawRect(m_rect);
        drawHandles(p);
    }
//    if (!m_Playing) {
        if (m_MD) {
            QRect r = m_sourceRect;
            Qt::KeyboardModifiers kb = qApp->queryKeyboardModifiers();
            if ((kb == Qt::NoModifier) || (m_activeHandle != NoHandle)) r = QRectF(pos() + m_rect.topLeft(),m_rect.size()).toRect();
            const QString posString = QString::number(r.left()) + "," + QString::number(r.top());
            const QString sizeString = QString::number(r.width()) + "," + QString::number(r.height());
            p->setBrush(Qt::black);
            p->setPen(QPen(Qt::white, 2));
            p->setFont(QFont("",12));
            p->drawText(m_rect.topLeft() + QPoint(10,17),posString);
            p->drawText(m_rect.bottomRight() - QPoint(60,2),sizeString);
            if (kb & (Qt::ShiftModifier | Qt::ControlModifier)) {
                p->setPen(QPen(Qt::yellow,1,Qt::DashLine));
                qreal scaleX = (qreal)m_frameSize.width() / m_rect.width();
                qreal scaleY = (qreal)m_frameSize.height() / m_rect.height();
                QRect scaledSource = QRect(QPoint(0,0),m_frameSize);
                if (qAbs(m_sourceRect.center().x() - scaledSource.center().x()) < scaleX) {
                    p->drawLine(m_rect.center().x(),m_rect.top(),m_rect.center().x(),m_rect.bottom());
                }
                if (qAbs(m_sourceRect.center().y() - scaledSource.center().y()) < scaleY) {
                    p->drawLine(m_rect.left(),m_rect.center().y(),m_rect.right(),m_rect.center().y());
                }
            }
        }
        else {
            if (!name.isEmpty()) {
                p->setBrush(Qt::black);
                p->setPen(QPen(Qt::yellow, 2));
                p->setFont(QFont("",12));
                p->drawText(m_rect.topLeft() + QPoint(10,17),name);
            }
        }
//    }
}

void CVideoItem::setThumbnail(const QImage& pix)
{
    prepareGeometryChange();
    thumbnail = pix;
    m_frameSize = pix.size() / pix.devicePixelRatio();
    if (m_frameSize.isEmpty()) m_frameSize = QSize(320,240);
    m_rect = QRect(QPoint(0,0),m_frameSize);
    m_sourceRect = m_rect;
    update();
}

void CVideoItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    if (m_Playing) return;
    m_MD = true;
    m_activeHandle = handleAt(e->pos().toPoint());
    m_pressPos = e->pos();
    m_pressSourceRect = m_sourceRect;
    scaleX = m_sourceRect.width()  / m_rect.width();
    scaleY = m_sourceRect.height() / m_rect.height();

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
    if (m_MD) {
        QPointF delta = e->pos() - m_pressPos;
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
                qreal ratio = (qreal)m_frameSize.width() / m_frameSize.height();

                m_rect.setHeight((qreal)m_rect.width() / ratio);
            }
            auto views = scene()->views();
            if (!views.isEmpty()) {
                if (auto v = qobject_cast<CVideoDesigner*>(views.first())) {
                    v->drawGuideLines(this);
                }
            }
            update();
            return;
        }
        if (e->modifiers() & Qt::ControlModifier)
        {
            QRectF r = m_pressSourceRect;

            r.setBottomRight(m_pressSourceRect.bottomRight() - scaledDelta);

            if (e->modifiers() & Qt::ShiftModifier) {
                qreal ratio = (qreal)m_rect.width() / m_rect.height();
                r.setHeight((qreal)r.width() / ratio);
            }

            m_sourceRect = r.toRect();
            snapSourceRect();
            update();
            return;
        }
        if (e->modifiers() & Qt::ShiftModifier)
        {
            QRectF r = m_pressSourceRect;
            r.moveTopLeft(m_pressSourceRect.topLeft() - scaledDelta);

            m_sourceRect = r.toRect();
            snapSourceRect();
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

void CVideoItem::pinchTriggered(QPinchGesture *gesture)
{
    if (m_Playing) return;
    static QRect startRect;
    static QRectF startSource;
    if (gesture->state() == Qt::GestureStarted) {
        m_MD = true;
        startRect = m_rect;
        startSource = m_sourceRect;
        if (scene()) scene()->clearSelection();
        setSelected(true);
    }
    if (gesture->state() == Qt::GestureUpdated)
    {
        bool shift = (qApp->queryKeyboardModifiers() == Qt::ShiftModifier);
        if (shift)
        {
            m_sourceRect.setSize(QSizeF(startSource.size() * gesture->totalScaleFactor()).toSize());
            snapSourceRect();
        }
        else
        {
            prepareGeometryChange();
            m_rect.setSize(startRect.size() * gesture->totalScaleFactor());
            auto views = scene()->views();
            if (!views.isEmpty()) {
                if (auto v = qobject_cast<CVideoDesigner*>(views.first())) {
                    v->drawGuideLines(this);
                }
            }
        }
    }
    if ((gesture->state() == Qt::GestureFinished) || (gesture->state() == Qt::GestureCanceled)) m_MD = false;
    update();
}

CVideoDesigner::CVideoDesigner(QWidget* parent)
    : QGraphicsView(parent)
{
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setBackgroundBrush(Qt::black);
    //setViewport(new QOpenGLWidget());
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    viewport()->setAttribute(Qt::WA_NoSystemBackground);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontSavePainterState);

    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setRenderHint(QPainter::Antialiasing, false);

    m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(&m_scene);

    setDragMode(QGraphicsView::RubberBandDrag);
    setTransformationAnchor(AnchorUnderMouse);
    m_scene.setSceneRect(0, 0, 1280, 720); // default canvas
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
