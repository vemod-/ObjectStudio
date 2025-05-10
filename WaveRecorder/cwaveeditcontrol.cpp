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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setEnabled(false);

    connect(zoomer,&QGraphicsViewZoomer::ZoomChanged,this,&CWaveEditControl::setZoom);
    connect(horizontalScrollBar(),&QAbstractSlider::valueChanged,this,&CWaveEditControl::Paint);

    //Zoom=0.01;
    //isMinZoom=false;
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

    //grabGesture(Qt::PinchGesture);
    //connect(this,&QViewportCanvas::paintRequested,this,&CWaveEditControl::Paint);
    setStyleSheet("QGraphicsView{background-color: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #ddd, stop: 1 #999);}");

}

CWaveEditControl::~CWaveEditControl()
{
    delete ui;
}

void CWaveEditControl::Init(CWaveGenerator *WG, CWaveGenerator::LoopParameters LP,bool LoopOn)
{
    m_LP=LP;
    m_WG=WG;
    m_LoopOn=LoopOn;
    if (m_Buffer != m_WG->channelPointer(0))
    {
        m_Buffer=m_WG->channelPointer(0);
        m_Length=m_WG->size();
        ZoomMin();
    }
    else
    {
        Paint();
    }
}
/*
bool CWaveEditControl::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        return gestureEvent(dynamic_cast<QGestureEvent*>(event));
    return QViewportCanvas::event(event);
}

bool CWaveEditControl::gestureEvent(QGestureEvent *event)
{
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
        pinchTriggered(dynamic_cast<QPinchGesture *>(pinch));
    return true;
}

void CWaveEditControl::pinchTriggered(QPinchGesture *gesture)
{
    double MinZoom=double(viewportSize().width())/m_Length;
    if (MinZoom>1) MinZoom=1;
    if ((gesture->scaleFactor()<1) && (Zoom*gesture->scaleFactor()<MinZoom)) return;
    if ((gesture->scaleFactor()>1) && (Zoom*gesture->scaleFactor()>1)) return;
    QPoint p(mapFromGlobal(QCursor::pos())+viewportPos());
    ulong64 Start=BufferPos(p.x());
    Zoom*=gesture->scaleFactor();
    if (Zoom>1) Zoom=1;
    double diff = mapFromGlobal(QCursor::pos()).x();
    setViewportLeft(PicPos(Start)-diff);
}
*/
void CWaveEditControl::Draw(CWaveGenerator::LoopParameters LP)
{
    m_LP=LP;
    if (Enabled)
    {
        if (visibleRect().height())
        {
            Paint();
        }
    }
}

void CWaveEditControl::Paint()
{
    Scene.clear();
    QRect r = viewport()->rect();
    r.setWidth(m_Length*zoomer->getZoom());
    setSceneRect(r);

    zoomer->setMin(ldouble(viewport()->width())/m_Length);
/*
    QLinearGradient lg(sceneRect().topLeft(),QPointF(sceneRect().width(),sceneRect().height()));
    lg.setColorAt(0,"#ddd");
    lg.setColorAt(1,"#999");
    setBackgroundBrush(lg);
*/
    DrawWave();
    m_StartLine = Scene.addLine(0,0,0,0,QPen(Qt::red));
    m_EndLine = Scene.addLine(0,0,0,0,QPen(Qt::red));
    m_LoopStartLine = Scene.addLine(0,0,0,0,QPen(Qt::red));
    m_LoopEndLine = Scene.addLine(0,0,0,0,QPen(Qt::red));
    m_AttackLine = Scene.addLine(0,0,0,0,QPen(Qt::red));
    m_SustainLine = Scene.addLine(0,0,0,0,QPen(Qt::red));
    m_ReleaseLine = Scene.addLine(0,0,0,0,QPen(Qt::red));

    m_Point1 = Scene.addEllipse(QRect(),QPen(Qt::red));
    m_Point2 = Scene.addEllipse(QRect(),QPen(Qt::red));

    DrawLines(m_LP,m_LoopOn);

    verticalScrollBar()->setEnabled(false);
    setStyleSheet("QGraphicsView{background-color: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #ddd, stop: 1 #999);}");

}

void CWaveEditControl::DrawLines(CWaveGenerator::LoopParameters LP, bool LoopOn)
{
    DrawLine(m_StartLine,LP.Start);
    DrawLine(m_EndLine,LP.End);
    if (LoopOn)
    {
        DrawLine(m_LoopStartLine,LP.LoopStart);
        DrawLine(m_LoopEndLine,LP.LoopEnd);
    }
    else
    {
        int Vol=Vol2Pos(LP.Volume);
        m_AttackLine->setLine(PicPos(LP.Start),Vol2Pos(0),PicPos(LP.Start+LP.FadeIn),Vol);
        m_SustainLine->setLine(PicPos(LP.Start+LP.FadeIn),Vol,PicPos(LP.End-LP.FadeOut),Vol);
        m_ReleaseLine->setLine(PicPos(LP.End-LP.FadeOut),Vol,PicPos(LP.End),Vol2Pos(0));

        m_Point1->setRect(QRect(PicPos(LP.Start+LP.FadeIn),Vol,6,6).translated(-3,-3));
        m_Point2->setRect(QRect(PicPos(LP.End-LP.FadeOut),Vol,6,6).translated(-3,-3));
    }
    //update();
}

void CWaveEditControl::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    /*
    double MinZoom=double(visibleRect().width())/m_Length;
    if (MinZoom > 1) MinZoom = 1;
    zoomer->setMin(MinZoom);
    if (isMinZoom)
    {
        ZoomMin();
    }
    else
    {
*/
        Paint();
    //}
}

void CWaveEditControl::showEvent(QShowEvent* e) {
    QGraphicsView::showEvent(e);
    Paint();
    ZoomMin();
}

void CWaveEditControl::DrawWave()
{
    /*
    double XInc=0.1;
    double PosInc=XInc/Zoom;
    if (PosInc < 1.0)
    {
        PosInc=1;
        XInc=PosInc*Zoom;
    }

    qDebug() << PosInc << XInc << Zoom;
*/
    //const int Right=viewPortGeometry().right();
    //if (canvasRect.size()!=NewSize) setSize(NewSize);
    //clearGradient();
    //Scene.clear();
    //setPen(Qt::black);
    /*
    QPen p(Qt::black);
    if (m_Length)
    {
        const CChannelBuffer* Buffer=m_WG->buffer();
        if (Buffer)
        {

            for (uint Channel=0;Channel<m_WG->channels();Channel++)
            {
                const float YFactor=sceneRect().height()/(2*m_WG->channels());
                const int HalfHeight=int (YFactor+(YFactor*Channel*2));
                ldouble X=viewPortGeometry().left();
                ldouble Pos=BufferPos(X);
                //moveTo(X,HalfHeight+(Buffer->at(Pos,Channel)*YFactor));
                while (X<Right)
                {
                    //moveTo(X,HalfHeight);
                    //lineTo(X,HalfHeight+(Buffer->at(Pos,Channel)*YFactor));
                    Scene.addLine(X,HalfHeight,X,HalfHeight+(Buffer->at(Pos,Channel)*YFactor),p);
                    Pos+=PosInc;
                    if (Pos>m_Length) break;
                    X+=XInc;
                }
            }
*/
    /*
            ldouble ZoomValue=1.0/Zoom;
            QRect r = viewPortGeometry();
            QPen pen(Qt::black);
            for (uint Channel=0;Channel<m_WG->channels();Channel++)
            {
                float YFactor=sceneRect().height()/(2*m_WG->channels());
                int HalfHeight=YFactor+(YFactor*Channel*2);
                ldouble i = ZoomValue*(r.left());
                //Canvas->moveTo(r.left(),HalfHeight);
                long zeroCount = 0;
                long i1;
                for (i1=r.left();i1<r.right();i1++)
                {
                    //if (i>=loopParameters.End) break;
                    if (i>=Buffer->size()) break;
                    int Val=Buffer->at(i,Channel)*YFactor;
                    if (Val==0)
                    {
                        zeroCount++;
                        //Canvas->lineTo(i1,HalfHeight);
                        //Scene.addLine(i1,HalfHeight,i1,HalfHeight,pen);
                    }
                    else
                    {
                        if (zeroCount) Scene.addLine(i1-zeroCount,HalfHeight,i1,HalfHeight);
                        zeroCount = 0;
                        Scene.addLine(i1,HalfHeight,i1,HalfHeight+Val,pen);
                        //Canvas->moveTo(i1,HalfHeight-Val);
                        //Canvas->lineTo(i1,HalfHeight+Val);
                    }
                    i=i+ZoomValue;
                }
                if (zeroCount) Scene.addLine(i1-zeroCount,HalfHeight,i1-1,HalfHeight);
            }

        }
    }
*/
    CWaveGenerator::LoopParameters LP;
    LP.End = m_WG->size();
    m_WG->paint(Scene,sceneRect().toRect(), visibleRect(), zoomer->getZoom(), &LP);
}

void inline CWaveEditControl::DrawLine(QGraphicsLineItem* l, ulong64 Position)
{
    if (Position<=m_Length)
    {
        //QViewportCanvasLayer* L=canvasLayers[0];
        //L->setPen(Qt::red);
        int Pos=PicPos(Position);
        //L->drawLine(Pos,Vol2Pos(0),Pos,Vol2Pos(100));
        l->setLine(Pos,Vol2Pos(0),Pos,Vol2Pos(100));
    }
}

void CWaveEditControl::ZoomOut()
{
    ulong64 s=BufferPos(visibleRect().left());
    /*
    isMinZoom=false;
    Zoom=Zoom*0.5;
    if (Zoom<zoomer->min())
    {
        Zoom=zoomer->min();
        isMinZoom=true;
    }
*/
    setZoom(zoomer->getZoom()*0.5);
    scrollToPos(PicPos(s));
}

void CWaveEditControl::ZoomIn()
{
    ulong64 s=BufferPos(visibleRect().left());
    /*
    Zoom=Zoom*2;
    if (Zoom>1) Zoom=1;
    isMinZoom=false;
*/
    setZoom(zoomer->getZoom()*2);
    scrollToPos(PicPos(s));
}

void CWaveEditControl::ZoomMin()
{
    //Zoom=zoomer->min();
    //isMinZoom=true;
    setZoom(zoomer->min());
    scrollToPos(0);
}

void CWaveEditControl::ZoomMax()
{
    ulong64 s=BufferPos(visibleRect().left());
    //Zoom=1;
    //isMinZoom=false;
    setZoom(1);
    scrollToPos(PicPos(s));
}

void CWaveEditControl::setZoom(double z) {
    double s = double(horizontalScrollBar()->maximum())/horizontalScrollBar()->value();
    if (z > 1) z = 1;
    if (z != zoomer->getZoom()) zoomer->setZoom(z);
    //Zoom = z;
    Paint();
    horizontalScrollBar()->setValue(horizontalScrollBar()->maximum()/s);
}

ulong64 CWaveEditControl::BufferPos(int X)
{
    ulong64 Pos=ldouble(X)/zoomer->getZoom();
    if (Pos>m_Length) return m_Length;
    return Pos;
}

int CWaveEditControl::PicPos(ldouble Position)
{
    return Position*zoomer->getZoom();
}

int CWaveEditControl::Vol2Pos(int Vol)
{
    float Height=viewport()->height();
    return Height-(Height*Vol*0.01);
}

int CWaveEditControl::Pos2Vol(int Pos)
{
    float Height=viewport()->height();
    int Vol= ((Height-Pos)*100)/Height;
    if (Vol<0)
    {
        Vol=0;
    }
    if (Vol>100)
    {
        Vol=100;
    }
    return Vol;
}

void CWaveEditControl::scrollToPos(int Start)
{
    //moveViewport(QPoint(Start,0));
    horizontalScrollBar()->setValue(Start);
}

void inline CWaveEditControl::MoveWaveLines(QPointF p)
{
    ulong64 Pos=BufferPos(p.x());
    if (Pos>m_Length) Pos=m_Length;
    if (WaveLines==wlStart)
    {
        if (m_LoopOn)
        {
            if (Pos>m_LP.LoopStart) Pos=m_LP.LoopStart;
        }
        else
        {
            if (Pos>m_LP.End-(m_LP.FadeIn+m_LP.FadeOut)) Pos=m_LP.End-(m_LP.FadeIn+m_LP.FadeOut);
        }
        m_LP.Start=Pos;
    }
    else if (WaveLines==wlEnd)
    {
        if (m_LoopOn)
        {
            if (Pos<m_LP.LoopEnd) Pos=m_LP.LoopEnd;
        }
        else
        {
            if (Pos<m_LP.Start+m_LP.FadeIn+m_LP.FadeOut) Pos=m_LP.Start+m_LP.FadeIn+m_LP.FadeOut;
        }
        m_LP.End=Pos;
    }
    else if (WaveLines==wlLoopStart)
    {
        if (Pos<m_LP.Start) Pos=m_LP.Start;
        if (Pos>m_LP.LoopEnd) Pos=m_LP.LoopEnd;
        m_LP.LoopStart=Pos;
    }
    else if (WaveLines==wlLoopEnd)
    {
        if (Pos<m_LP.LoopStart) Pos=m_LP.LoopStart;
        if (Pos>m_LP.End) Pos=m_LP.End;
        m_LP.LoopEnd=Pos;
    }
    else if (WaveLines==wlFadeIn)
    {
        if (Pos<m_LP.Start) Pos=m_LP.Start;
        if (Pos>m_LP.End-m_LP.FadeOut) Pos=m_LP.End-m_LP.FadeOut;
        m_LP.FadeIn=Pos-m_LP.Start;
        m_LP.Volume=Pos2Vol(p.y());
    }
    else if (WaveLines==wlFadeOut)
    {
        if (Pos<m_LP.Start+m_LP.FadeIn) Pos=m_LP.Start+m_LP.FadeIn;
        if (Pos>m_LP.End) Pos=m_LP.End;
        m_LP.FadeOut=m_LP.End-Pos;
        m_LP.Volume=Pos2Vol(p.y());
    }
    DrawLines(m_LP,m_LoopOn);
    emit ParameterChanged(m_LP);
}

void CWaveEditControl::mousePressEvent(QMouseEvent* /*event*/)
{
    MD=true;
}

bool inline InsideLine(int X,int LineX)
{
    return (X>=LineX-4 && X<=LineX+4);
}

void CWaveEditControl::mouseMoveEvent(QMouseEvent *event)
{

    if (Enabled)
    {
        //QPoint Pos(event->pos()+viewportPos());
        QPointF Pos = mapToScene(event->pos());
        if (!MD)
        {
            if (m_LoopOn)
            {
                if (InsideLine(Pos.x(),PicPos(m_LP.Start)))
                {
                    setCursor(Qt::SizeHorCursor);
                    WaveLines=wlStart;
                }
                else if (InsideLine(Pos.x(),PicPos(m_LP.End)))
                {
                    setCursor(Qt::SizeHorCursor);
                    WaveLines=wlEnd;
                }
                else if (InsideLine(Pos.x(),PicPos(m_LP.LoopStart)))
                {
                    setCursor(Qt::SizeHorCursor);
                    WaveLines=wlLoopStart;
                }
                else if (InsideLine(Pos.x(),PicPos(m_LP.LoopEnd)))
                {
                    setCursor(Qt::SizeHorCursor);
                    WaveLines=wlLoopEnd;
                }
                else
                {
                    setCursor(Qt::ArrowCursor);
                    WaveLines=wlNone;
                }
            }
            else
            {
                if ((InsideLine(Pos.x(),PicPos(m_LP.Start+m_LP.FadeIn))) &&  (InsideLine(Pos.y(),Vol2Pos(m_LP.Volume))))
                {
                    setCursor(Qt::SizeAllCursor);
                    WaveLines=wlFadeIn;
                }
                else if ((InsideLine(Pos.x(),PicPos(m_LP.End-m_LP.FadeOut))) &&  (InsideLine(Pos.y(),Vol2Pos(m_LP.Volume))))
                {
                    setCursor(Qt::SizeAllCursor);
                    WaveLines=wlFadeOut;
                }
                else if (InsideLine(Pos.x(),PicPos(m_LP.Start)))
                {
                    setCursor(Qt::SizeHorCursor);
                    WaveLines=wlStart;
                }
                else if (InsideLine(Pos.x(),PicPos(m_LP.End)))
                {
                    setCursor(Qt::SizeHorCursor);
                    WaveLines=wlEnd;
                }
                else
                {
                    setCursor(Qt::ArrowCursor);
                    WaveLines=wlNone;
                }
            }
        }
        else
        {
            if ((WaveLines==wlFadeIn) || (WaveLines==wlFadeOut))
            {
                if (Pos != OldPos)
                {
                    MoveWaveLines(Pos);
                    OldPos=Pos;
                }
            }
            else
            {
                if (Pos.x() != OldPos.x())
                {
                    MoveWaveLines(Pos);
                    OldPos.setX(Pos.x());
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void CWaveEditControl::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF Pos = mapToScene(event->pos());
    if (Enabled)
    {
        if (MD)
        {
            MoveWaveLines(Pos);
        }
    }
    MD=false;
}
