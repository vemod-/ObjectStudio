#include "cadsrcontrol.h"
#include "ui_cadsrcontrol.h"
#include <QGraphicsLineItem>
#include <QMouseEvent>

#define ADSR_FrameWidth 4

CADSRControl::CADSRControl(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::CADSRControl)
{
    ui->setupUi(this);
    MD=false;
    SplitValue=svADSRNone;
    setMouseTracking(true);
    ReleaseStart = ADSRControl::ADSR_MinWidth*5;
    MaxWidth = ADSRControl::ADSR_MinWidth*6;
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

    //connect(this,&QViewportCanvas::paintRequested,this,&CADSRControl::Paint);
}

CADSRControl::~CADSRControl()
{
    delete ui;
}

int inline CADSRControl::Time2X(const ldouble Time)
{
    return ((Time*(sceneRect().width()-(ADSR_FrameWidth*2)))/MaxWidth)+ADSR_FrameWidth;
}

int inline CADSRControl::Vol2Y(const int Vol)
{
    return ((120-Vol)*(sceneRect().height()-(ADSR_FrameWidth*2))/120)+ADSR_FrameWidth;
}

ldouble inline CADSRControl::X2Time(const int X)
{
    return ldouble((X-ADSR_FrameWidth)*MaxWidth)/ldouble(sceneRect().width()-(ADSR_FrameWidth*2));
}

int inline CADSRControl::Y2Vol(const int Y)
{
    return 120-(((Y-ADSR_FrameWidth)*120)/(sceneRect().height()-(ADSR_FrameWidth*2)));
}

void CADSRControl::Paint()
{
    Draw(ADSR.AP);
}

void CADSRControl::Draw(CADSR::ADSRParams ADSRParams)
{
    ADSR.AP=ADSRParams;
    ulong64 i=ADSRControl::ADSR_MinWidth*5;
    if (ADSR.AP.Delay > ADSRControl::ADSR_MinWidth) i += (ADSR.AP.Delay-ADSRControl::ADSR_MinWidth);
    if (ADSR.AP.Attack > ADSRControl::ADSR_MinWidth) i += (ADSR.AP.Attack-ADSRControl::ADSR_MinWidth);
    if (ADSR.AP.Hold > ADSRControl::ADSR_MinWidth) i += (ADSR.AP.Hold-ADSRControl::ADSR_MinWidth);
    if (ADSR.AP.Decay > ADSRControl::ADSR_MinWidth) i += (ADSR.AP.Decay-ADSRControl::ADSR_MinWidth);
    ReleaseStart=i;
    i+=ADSRControl::ADSR_MinWidth;
    if (ADSR.AP.Release > ADSRControl::ADSR_MinWidth) i += (ADSR.AP.Release-ADSRControl::ADSR_MinWidth);
    MaxWidth=i+200;
    Scene.clear();
    QRect r = viewport()->rect();
    r.setSize(QSize((((viewport()->width()-(ADSR_FrameWidth*2))*MaxWidth)/(MaxWidth))+(ADSR_FrameWidth*2),viewport()->height()));
    setSceneRect(r);

    ADSR.Start();
    //clearGradient();
    //setPen(Qt::black);
    Pen = QPen(Qt::black);
    moveTo(Time2X(0),Vol2Y(0));
    for (ulong64 i=0;i<=ADSR.mSec2Buffer(ADSR.AP.Delay+ADSR.AP.Attack+ADSR.AP.Hold+ADSR.AP.Decay);i++)
    {
        lineTo(Time2X(ADSR.Buffer2mSec(i)),Vol2Y(ADSR.GetVol()*100));
    }
    lineTo(Time2X(ADSR.AP.Delay+ADSR.AP.Attack+ADSR.AP.Hold+ADSR.AP.Decay),Vol2Y(ADSR.AP.Sustain));

    Pen = QPen(Qt::darkGray,0,Qt::DashLine);

    Scene.addLine(Time2X(ADSR.AP.Delay+ADSR.AP.Attack+ADSR.AP.Hold+ADSR.AP.Decay),Vol2Y(ADSR.AP.Sustain),Time2X(ReleaseStart),Vol2Y(ADSR.AP.Sustain));
    moveTo(Time2X(ReleaseStart),Vol2Y(ADSR.AP.Sustain));
    Pen = QPen(Qt::black);
    ADSR.Finish();
    for (ulong64 i=ADSR.mSec2Buffer(ReleaseStart);i<=ADSR.mSec2Buffer(ReleaseStart+ADSR.AP.Release);i++)
    {
        lineTo(Time2X(ADSR.Buffer2mSec(i)),Vol2Y(ADSR.GetVol()*100));
    }
    Pen = QPen(Qt::red);
    //setPenBrush(Qt::red);
    //setBrush(QBrush(Qt::NoBrush));

    drawCircle(Time2X(ADSR.AP.Delay),Vol2Y(0),3);
    drawCircle(Time2X(ADSR.AP.Delay+ADSR.AP.Attack),Vol2Y(100),3);
    drawCircle(Time2X(ADSR.AP.Delay+ADSR.AP.Attack+ADSR.AP.Hold),Vol2Y(100),3);
    drawCircle(Time2X(ADSR.AP.Delay+ADSR.AP.Attack+ADSR.AP.Hold+ADSR.AP.Decay),Vol2Y(ADSR.AP.Sustain),3);
    drawCircle(Time2X(ReleaseStart),Vol2Y(ADSR.AP.Sustain),3);
    drawCircle(Time2X(ReleaseStart+ADSR.AP.Release),Vol2Y(0),3);

    //update();
}

bool InsidePoint(const int X,const int Y,const QPoint P)
{
    return QRect(X,Y,0,0).adjusted(-4,-4,4,4).contains(P);
}

void inline CADSRControl::MoveLines(QPoint P)
{
    P.setY(Y2Vol(P.y()));
    P.setX(X2Time(P.x()));
    PutPoint(P,SplitValue);
}

void inline CADSRControl::PutPoint(const QPoint P,const SplitterValuesADSR SV)
{
    if (SV==svDecay)
    {
        int D=P.x()-(ADSR.AP.Delay+ADSR.AP.Attack+ADSR.AP.Hold);
        D = qBound<int>(0,D,ADSRControl::ADSR_MaxTime);
        ADSR.AP.Decay=D;
        const int Vol=qBound<int>(0,P.y(),100);
        ADSR.AP.Sustain=Vol;
        Draw(ADSR.AP);
        emit DecayChanged(D);
        emit SustainChanged(Vol);
        emit Changed(ADSR.AP);
    }
    else if (SV==svSustain)
    {
        const int Vol=qBound<int>(0,P.y(),100);
        ADSR.AP.Sustain=Vol;
        Draw(ADSR.AP);
        emit SustainChanged(Vol);
        emit Changed(ADSR.AP);
    }
    else if (SV==svAttack)
    {
        int D=P.x()-ADSR.AP.Delay;
        D = qBound<int>(0,D,ADSRControl::ADSR_MaxTime);
        ADSR.AP.Attack=D;
        Draw(ADSR.AP);
        emit AttackChanged(ADSR.AP.Attack);
        emit Changed(ADSR.AP);
    }
    else if (SV==svHold)
    {
        int D=P.x()-(ADSR.AP.Delay+ADSR.AP.Attack);
        D = qBound<int>(0,D,ADSRControl::ADSR_MaxTime);
        ADSR.AP.Hold=D;
        Draw(ADSR.AP);
        emit HoldChanged(ADSR.AP.Hold);
        emit Changed(ADSR.AP);
    }
    else if (SV==svRelease)
    {
        int D=P.x()-ReleaseStart;
        D = qBound<int>(0,D,ADSRControl::ADSR_MaxTime);
        ADSR.AP.Release=D;
        Draw(ADSR.AP);
        emit ReleaseChanged(ADSR.AP.Release);
        emit Changed(ADSR.AP);
    }
    else if (SV==svDelay)
    {
        if ((P.x()>=0) && (P.x()<=ADSRControl::ADSR_MaxTime))
        {
            ADSR.AP.Delay=P.x();
            Draw(ADSR.AP);
            emit DelayChanged(ADSR.AP.Delay);
            emit Changed(ADSR.AP);
        }
    }

}

void CADSRControl::mousePressEvent(QMouseEvent* /*event*/)
{
    MD=true;
}

void CADSRControl::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint P = mapToScene(event->pos()).toPoint();
    if (!MD)
    {
        if (InsidePoint(Time2X(ADSR.AP.Delay),Vol2Y(0),P))
        {
            setCursor(Qt::SizeHorCursor);
            SplitValue=svDelay;
        }
        else if (InsidePoint(Time2X(ADSR.AP.Delay+ADSR.AP.Attack),Vol2Y(100),P))
        {
            setCursor(Qt::SizeHorCursor);
            SplitValue=svAttack;
        }
        else if (InsidePoint(Time2X(ADSR.AP.Delay+ADSR.AP.Attack+ADSR.AP.Hold),Vol2Y(100),P))
        {
            setCursor(Qt::SizeHorCursor);
            SplitValue=svHold;
        }
        else if (InsidePoint(Time2X(ADSR.AP.Delay+ADSR.AP.Attack+ADSR.AP.Hold+ADSR.AP.Decay),Vol2Y(ADSR.AP.Sustain),P))
        {
            setCursor(Qt::SizeAllCursor);
            SplitValue=svDecay;
        }
        else if (InsidePoint(Time2X(ReleaseStart),Vol2Y(ADSR.AP.Sustain),P))
        {
            setCursor(Qt::SizeVerCursor);
            SplitValue=svSustain;
        }
        else if (InsidePoint(Time2X(ReleaseStart+ADSR.AP.Release),Vol2Y(0),P))
        {
            setCursor(Qt::SizeHorCursor);
            SplitValue=svRelease;
        }
        else
        {
            setCursor(Qt::ArrowCursor);
            SplitValue=svADSRNone;
        }
    }
    else
    {
        if (P != OldPoint)
        {
            MoveLines(P);
            OldPoint=P;
        }
    }
}

void CADSRControl::mouseReleaseEvent(QMouseEvent *event)
{
    if (MD) if (SplitValue!=svADSRNone) MoveLines(mapToScene(event->pos()).toPoint());
    MD=false;
}

void CADSRControl::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    Paint();
}

void CADSRControl::setDelay(int v)
{
    ADSR.AP.Delay=v;
    Draw(ADSR.AP);
}

void CADSRControl::setAttack(int v)
{
    ADSR.AP.Attack=v;
    Draw(ADSR.AP);
}

void CADSRControl::setHold(int v)
{
    ADSR.AP.Hold=v;
    Draw(ADSR.AP);
}

void CADSRControl::setDecay(int v)
{
    ADSR.AP.Decay=v;
    Draw(ADSR.AP);
}

void CADSRControl::setSustain(int v)
{
    ADSR.AP.Sustain=v;
    Draw(ADSR.AP);
}

void CADSRControl::setRelease(int v)
{
    ADSR.AP.Release=v;
    Draw(ADSR.AP);
}
