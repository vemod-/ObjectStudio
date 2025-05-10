#include "ctimelineslider.h"
#include "ctimelineedit.h"

CTimeLineSlider::CTimeLineSlider(QWidget *parent)
    : QGraphicsView{parent}
{
    setScene(&Scene);
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(1);
    setMidLineWidth(2);
    setFixedHeight(timelineheight+4+(frameWidth()*2));
    setBackgroundBrush(Qt::darkGray);
    setScene(&Scene);
    setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    Scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    InfoLabel=new QLabel(parent);
    InfoLabel->setAutoFillBackground(true);
    InfoLabel->setFrameStyle(QFrame::Box | QFrame::Plain);
    InfoLabel->hide();
}

void CTimeLineSlider::resizeEvent(QResizeEvent* /*e*/) {
    m_TimeLine.setWidth(viewport()->width());
    setSceneRect(viewport()->rect());
    draw();
}

void CTimeLineSlider::timerEvent(QTimerEvent* /*e*/) {
    if (!m_TimerID) return;
    m_TimeLine.handleTimer(m_Device);
}

void CTimeLineSlider::mouseDoubleClickEvent(QMouseEvent* e) {
    QPointF p = QGraphicsView::mapToScene(e->pos());
    m_TimeLine.handleDoubleClick(p, m_Device);
}

void CTimeLineSlider::mousePressEvent(QMouseEvent* e) {
    QPointF p = QGraphicsView::mapToScene(e->pos());
    if (e->button() == Qt::RightButton) {
        CTimeLineMenu* d = new CTimeLineMenu(&m_TimeLine,this);
        connect(d,&CTimeLineMenu::Changed,this,&CTimeLineSlider::draw);
        d->popup(cursor().pos());
        return;
    }
    m_TimeLine.handleMousePress(p, m_Device);
}

void CTimeLineSlider::mouseMoveEvent(QMouseEvent* e) {
    QPointF p = QGraphicsView::mapToScene(e->pos());
    m_TimeLine.handleMouseMove(p, m_Device);
}

void CTimeLineSlider::mouseReleaseEvent(QMouseEvent* e) {
    QPointF p = QGraphicsView::mapToScene(e->pos());
    m_TimeLine.handleMouseRelease(p, m_Device);
}
