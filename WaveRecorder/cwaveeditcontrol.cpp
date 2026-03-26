#include "cwaveeditcontrol.h"
#include "ui_cwaveeditcontrol.h"

CWaveEditControl::CWaveEditControl(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::CWaveEditControl)
{
    ui->setupUi(this);
    MD=false;
    setMouseTracking(true);
    zoomer = new QGraphicsViewZoomer(this);
    zoomer->disableMatrix();
    zoomer->setMin(0.0001);
    zoomer->setMax(1);
    zoomer->setZoom(0.01);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setEnabled(false);

    connect(zoomer,&QGraphicsViewZoomer::ZoomChanged,this,&CWaveEditControl::ZoomToCursor);
    connect(horizontalScrollBar(),&QAbstractSlider::valueChanged,this,&CWaveEditControl::scrollbarUpdate);

    Enabled=false;
    m_Length=0;
    m_Buffer=nullptr;
    setScene(&Scene);
    setSceneRect(0,0,1,1);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    Scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
    setStyleSheet("QGraphicsView{background-color: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #ddd, stop: 1 #999);}");
    m_StartLine = Scene.addLine(0,0,0,0,QPen(Qt::red));
    m_StartLine->setZValue(1);
    m_EndLine = Scene.addLine(0,0,0,0,QPen(Qt::red));
    m_EndLine->setZValue(1);
    m_LoopStartLine = Scene.addLine(0,0,0,0,QPen(Qt::green));
    m_LoopStartLine->setZValue(1);
    m_LoopEndLine = Scene.addLine(0,0,0,0,QPen(Qt::green));
    m_LoopEndLine->setZValue(1);
    m_AttackLine = Scene.addLine(0,0,0,0,QPen(Qt::yellow));
    m_AttackLine->setZValue(1);
    m_SustainLine = Scene.addLine(0,0,0,0,QPen(Qt::yellow));
    m_SustainLine->setZValue(1);
    m_ReleaseLine = Scene.addLine(0,0,0,0,QPen(Qt::yellow));
    m_ReleaseLine->setZValue(1);
    m_Point1 = Scene.addEllipse(QRect(),QPen(Qt::yellow));
    m_Point1->setZValue(1);
    m_Point2 = Scene.addEllipse(QRect(),QPen(Qt::yellow));
    m_Point2->setZValue(1);
    m_VideoFadeInLine = Scene.addLine(0,0,0,0,QPen(Qt::blue));
    m_VideoFadeInLine->setZValue(1);
    m_VideoFadeOutLine = Scene.addLine(0,0,0,0,QPen(Qt::blue));
    m_VideoFadeOutLine->setZValue(1);
    m_VideoOpaqueLine = Scene.addLine(0,0,0,0,QPen(Qt::blue));
    m_VideoOpaqueLine->setZValue(1);
    m_VideoPoint1 = Scene.addEllipse(QRect(),QPen(Qt::blue));
    m_VideoPoint1->setZValue(1);
    m_VideoPoint2 = Scene.addEllipse(QRect(),QPen(Qt::blue));
    m_VideoPoint2->setZValue(1);
    m_StartLine->setCursor(Qt::SizeHorCursor);
    m_EndLine->setCursor(Qt::SizeHorCursor);
    m_LoopStartLine->setCursor(Qt::SizeHorCursor);
    m_LoopEndLine->setCursor(Qt::SizeHorCursor);
    m_SustainLine->setCursor(Qt::SizeVerCursor);
    m_Point1->setCursor(Qt::SizeAllCursor);
    m_Point2->setCursor(Qt::SizeAllCursor);
    m_VideoOpaqueLine->setCursor(Qt::SizeVerCursor);
    m_VideoPoint1->setCursor(Qt::SizeAllCursor);
    m_VideoPoint2->setCursor(Qt::SizeAllCursor);
}

CWaveEditControl::~CWaveEditControl()
{
    delete ui;
}

void CWaveEditControl::Init(CWaveGenerator *WG, CWaveGenerator::LoopParameters LP,bool LoopOn)
{
    m_LP = LP;
    m_WG = WG;
    m_LoopOn = LoopOn;
    m_Length = m_WG->size();
    if (m_Length == 0) m_Length = LP.End;
    m_Buffer = m_WG->channelPointer(0);
    m_StartLine->setVisible(!m_WG->hasImage());
    m_LoopStartLine->setVisible(m_LoopOn && !m_WG->hasImage());
    m_LoopEndLine->setVisible(m_LoopOn && !m_WG->hasImage());
    m_AttackLine->setVisible(!m_LoopOn && !m_WG->hasImage());
    m_SustainLine->setVisible(!m_LoopOn && !m_WG->hasImage());
    m_ReleaseLine->setVisible(!m_LoopOn && !m_WG->hasImage());
    m_Point1->setVisible(!m_LoopOn && !m_WG->hasImage());
    m_Point2->setVisible(!m_LoopOn && !m_WG->hasImage());
    m_VideoFadeInLine->setVisible(m_WG->hasVisual());
    m_VideoFadeOutLine->setVisible(m_WG->hasVisual());
    m_VideoOpaqueLine->setVisible(m_WG->hasVisual());
    m_VideoPoint1->setVisible(m_WG->hasVisual());
    m_VideoPoint2->setVisible(m_WG->hasVisual());
    ZoomMin();
}

void CWaveEditControl::Draw(CWaveGenerator::LoopParameters LP)
{
    m_LP=LP;
    if (Enabled) {
        if (zoomer->visibleRect().height() > 0) DrawLines(m_LP,m_LoopOn);
    }
}

void CWaveEditControl::Paint()
{
    if (m_WaveItem) {
        m_WaveItem->setParentItem(nullptr);
        delete m_WaveItem;
    }
    setSceneRect(0,0,m_Length * zoomer->getZoom(),viewport()->height());
    zoomer->setMin(ldouble(viewport()->width())/m_Length);
    m_WaveItem = DrawWave();
    if (m_WaveItem) Scene.addItem(m_WaveItem);
    DrawLines(m_LP,m_LoopOn);
    verticalScrollBar()->setEnabled(false);
    setStyleSheet("QGraphicsView{background-color: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #ddd, stop: 1 #999);}");
}

void CWaveEditControl::DrawLines(CWaveGenerator::LoopParameters LP, bool LoopOn)
{
    if (m_WG->size() > 0) DrawLine(m_StartLine,LP.Start);
    DrawLine(m_EndLine,LP.End);
    if (LoopOn) {
        DrawLine(m_LoopStartLine,LP.LoopStart);
        DrawLine(m_LoopEndLine,LP.LoopEnd);
    }
    else {
        const int VolPos = Vol2Pos(LP.Volume);
        m_AttackLine->setLine(SampleToPos(LP.Start),Vol2Pos(0),SampleToPos(LP.Start+LP.FadeIn),VolPos);
        m_SustainLine->setLine(SampleToPos(LP.Start+LP.FadeIn),VolPos,SampleToPos(LP.End-LP.FadeOut),VolPos);
        m_ReleaseLine->setLine(SampleToPos(LP.End-LP.FadeOut),VolPos,SampleToPos(LP.End),Vol2Pos(0));
        m_Point1->setRect(QRect(SampleToPos(LP.Start+LP.FadeIn),VolPos,6,6).translated(-3,-3));
        m_Point2->setRect(QRect(SampleToPos(LP.End-LP.FadeOut),VolPos,6,6).translated(-3,-3));
    }
    if (m_WG->hasVisual()) {
        const int OpaquePos = Vol2Pos(LP.VideoOpacity);
        m_VideoFadeInLine->setLine(SampleToPos(LP.Start),Vol2Pos(0),SampleToPos(LP.Start+LP.VideoFadeIn),OpaquePos);
        m_VideoOpaqueLine->setLine(SampleToPos(LP.Start+LP.VideoFadeIn),OpaquePos,SampleToPos(LP.End-LP.VideoFadeOut),OpaquePos);
        m_VideoFadeOutLine->setLine(SampleToPos(LP.End-LP.VideoFadeOut),OpaquePos,SampleToPos(LP.End),Vol2Pos(0));
        m_VideoPoint1->setRect(QRect(SampleToPos(LP.Start+LP.VideoFadeIn),OpaquePos,6,6).translated(-3,-3));
        m_VideoPoint2->setRect(QRect(SampleToPos(LP.End-LP.VideoFadeOut),OpaquePos,6,6).translated(-3,-3));
    }
}

void CWaveEditControl::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    Paint();
}

void CWaveEditControl::showEvent(QShowEvent* e) {
    QGraphicsView::showEvent(e);
    Paint();
    if (Region) {
        ZoomRegion();
    }
    else {
        ZoomMin();
    }
}

QGraphicsItem* CWaveEditControl::DrawWave()
{
    CWaveGenerator::LoopParameters LP;
    LP.End = m_Length;
    return m_WG->waveFormItem(sceneRect().toRect(), zoomer->visibleRect().toRect(), zoomer->getZoom(), &LP);
}

void inline CWaveEditControl::DrawLine(QGraphicsLineItem* l, ulong64 Sample)
{
    if (Sample <= m_Length) {
        const int Pos = SampleToPos(Sample);
        l->setLine(Pos,Vol2Pos(0),Pos,Vol2Pos(100));
    }
}

void CWaveEditControl::ZoomOut()
{
    setZoom(zoomer->getZoom() * 0.5);
}

void CWaveEditControl::ZoomIn()
{
    setZoom(zoomer->getZoom() * 2);
}

void CWaveEditControl::ZoomMin()
{
    setZoom(zoomer->min());
}

void CWaveEditControl::ZoomMax()
{
    setZoom(1);
}

void CWaveEditControl::setZoom(double z) {
    ZoomToPoint(z,viewport()->rect().center());
}

void CWaveEditControl::ZoomToPoint(double z, const QPointF p) {
    z = std::clamp(z,zoomer->min(),zoomer->max());
    const ulong64 s = PosToSample(zoomer->visibleRect().left() + p.x());
    if (!closeEnough(z,zoomer->getZoom())) zoomer->setZoom(z);
    setSceneRect(0,0,m_Length * zoomer->getZoom(),viewport()->height());
    scrollToSample(s - PosToSample(p.x()));
}

void CWaveEditControl::ZoomToCursor(double z, double o) {
    const QPointF p = viewport()->mapFromGlobal(QCursor::pos());
    const ulong64 s = (zoomer->visibleRect().left() + p.x()) / o;
    setSceneRect(0,0,m_Length * z,viewport()->height());
    scrollToSample(s - PosToSample(p.x()));
}

void CWaveEditControl::ZoomRegion() {
    if (m_LP.Start == 0) {
        if (m_LP.End == m_Length) {
            ZoomMin();
            return;
        }
    }
    ldouble visibleSamples = m_LP.End - m_LP.Start;
    if (visibleSamples <= 0) {
        ZoomMin();
        return;
    }
    setZoom(viewport()->width() / visibleSamples);
    scrollToSample(m_LP.Start);
}

ulong64 CWaveEditControl::PosToSample(double X)
{
    ulong64 Sample = X / zoomer->getZoom();
    if (Sample > m_Length) return m_Length;
    return Sample;
}

double CWaveEditControl::SampleToPos(ulong64 Sample)
{
    return Sample * zoomer->getZoom();
}

int CWaveEditControl::Vol2Pos(int Vol)
{
    float Height = viewport()->height();
    return Height - (Height * Vol * 0.01);
}

int CWaveEditControl::Pos2Vol(int Pos)
{
    float Height = viewport()->height();
    return std::clamp<int>(((Height - Pos) * 100.0) / Height,0,100);
}

void CWaveEditControl::scrollToPos(double x)
{
    qDebug() << "moved by zoom" << x;
    noScrollbarUpdate = true;
    zoomer->scrollXTo(x);
    noScrollbarUpdate = false;
    Paint();
}

void CWaveEditControl::scrollbarUpdate(int x){
    if (noScrollbarUpdate) return;
    qDebug() << "moved by scrollbar signal" << x;
    Paint();
}

void CWaveEditControl::scrollToSample(long64 s) {
    scrollToPos(s * zoomer->getZoom());
}

void inline CWaveEditControl::MoveWaveLines(QPointF p)
{
    ulong64 Pos = PosToSample(p.x());
    if (Pos > m_Length) Pos = m_Length;
    if (DragItem == m_StartLine) {
        if (m_LoopOn) {
            if (Pos>m_LP.LoopStart) Pos = m_LP.LoopStart;
        }
        else {
            if (Pos > m_LP.End - (m_LP.FadeIn + m_LP.FadeOut)) Pos = m_LP.End - (m_LP.FadeIn + m_LP.FadeOut);
        }
        m_LP.Start=Pos;
    }
    else if (DragItem == m_EndLine) {
        if (m_LoopOn) {
            if (Pos < m_LP.LoopEnd) Pos = m_LP.LoopEnd;
        }
        else {
            if (Pos < m_LP.Start + m_LP.FadeIn + m_LP.FadeOut) Pos = m_LP.Start + m_LP.FadeIn+m_LP.FadeOut;
        }
        m_LP.End = Pos;
    }
    else if (DragItem == m_LoopStartLine) {
        if (Pos < m_LP.Start) Pos = m_LP.Start;
        if (Pos > m_LP.LoopEnd) Pos = m_LP.LoopEnd;
        m_LP.LoopStart = Pos;
    }
    else if (DragItem == m_LoopEndLine) {
        if (Pos < m_LP.LoopStart) Pos = m_LP.LoopStart;
        if (Pos > m_LP.End) Pos = m_LP.End;
        m_LP.LoopEnd = Pos;
    }
    else if (DragItem == m_SustainLine) {
        m_LP.Volume = Pos2Vol(p.y());
    }
    else if (DragItem == m_Point1) {
        if (Pos < m_LP.Start) Pos = m_LP.Start;
        if (Pos > m_LP.End - m_LP.FadeOut) Pos = m_LP.End-m_LP.FadeOut;
        m_LP.FadeIn = Pos - m_LP.Start;
        m_LP.Volume = Pos2Vol(p.y());
    }
    else if (DragItem == m_Point2) {
        if (Pos < m_LP.Start + m_LP.FadeIn) Pos = m_LP.Start + m_LP.FadeIn;
        if (Pos > m_LP.End) Pos = m_LP.End;
        m_LP.FadeOut = m_LP.End - Pos;
        m_LP.Volume = Pos2Vol(p.y());
    }
    else if (DragItem == m_VideoOpaqueLine) {
        m_LP.VideoOpacity = Pos2Vol(p.y());
    }
    else if (DragItem == m_VideoPoint1) {
        if (Pos < m_LP.Start) Pos = m_LP.Start;
        if (Pos > m_LP.End - m_LP.VideoFadeOut) Pos = m_LP.End-m_LP.VideoFadeOut;
        m_LP.VideoFadeIn = Pos - m_LP.Start;
        m_LP.VideoOpacity = Pos2Vol(p.y());
    }
    else if (DragItem == m_VideoPoint2) {
        if (Pos < m_LP.Start + m_LP.FadeIn) Pos = m_LP.Start + m_LP.VideoFadeIn;
        if (Pos > m_LP.End) Pos = m_LP.End;
        m_LP.VideoFadeOut = m_LP.End - Pos;
        m_LP.VideoOpacity = Pos2Vol(p.y());
    }
    DrawLines(m_LP,m_LoopOn);
    emit ParameterChanged(m_LP);
}

void CWaveEditControl::mousePressEvent(QMouseEvent* event)
{
    if (Enabled)
    {
        MD=true;
        QPointF Pos = mapToScene(event->pos());
        QGraphicsItem* item = itemAt(Pos.toPoint());
        DragItem = nullptr;
        if (item->zValue() == 1) DragItem = item;
        qDebug() << item << (item == m_SustainLine);
    }
    QGraphicsView::mousePressEvent(event);
}

void CWaveEditControl::mouseMoveEvent(QMouseEvent *event)
{
    if (Enabled) {
        if (MD) {
            QPointF Pos = mapToScene(event->pos());
            MoveWaveLines(Pos);
        }
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CWaveEditControl::mouseReleaseEvent(QMouseEvent *event)
{
    if (Enabled)
    {
        if (MD)
        {
            QPointF Pos = mapToScene(event->pos());
            MoveWaveLines(Pos);
        }
    }
    MD=false;
    QGraphicsView::mouseReleaseEvent(event);
}
