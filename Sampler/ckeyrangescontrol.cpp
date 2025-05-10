#include "ckeyrangescontrol.h"
#include "ui_ckeyrangescontrol.h"

CKeyRangesControl::CKeyRangesControl(QWidget *parent) :
    QCanvas(parent,1),
    ui(new Ui::CKeyRangesControl)
{
    ui->setupUi(this);
    MD=false;
    setMouseTracking(true);
    m_Sampler=nullptr;
}

CKeyRangesControl::~CKeyRangesControl()
{
    delete ui;
}

void CKeyRangesControl::Init(CSamplerDevice *D)
{
    m_Sampler=D;
    Draw();
}

void CKeyRangesControl::resizeEvent(QResizeEvent* event)
{
    QCanvas::resizeEvent(event);
    Draw();
}

void inline CKeyRangesControl::MoveLines(QPoint P)
{
    if (m_Sampler->rangeCount()) PutPoint(m_Sampler->currentRange(),QPoint(Graph2X(P.x()),Graph2Vol(P.y())),SplitValue);
}

void inline CKeyRangesControl::PutPoint(CSampleKeyRange* Range,QPoint P,SplitterValues SV)
{
    if (SV==svUpper)
    {
        if ((P.x()-1>Range->parameters.LowerTop) && (P.x()-1<128))
        {
            Range->parameters.UpperTop=P.x()-1;
            Range->parameters.UpperZero = qBound<int>(P.x()-1, Range->parameters.UpperZero, 127);
            Range->parameters.Volume=qBound<int>(5, P.y(), 200);
            DrawLines();
            emit CurrentRangeChanged(Range->parameters);
        }
    }
    else if (SV==svLower)
    {
        if ((P.x()<Range->parameters.UpperTop) && (P.x()>=0))
        {
            Range->parameters.LowerTop=P.x();
            Range->parameters.LowerZero = qBound<int>(0, Range->parameters.LowerZero, P.x());
            Range->parameters.Volume=qBound<int>(5, P.y(), 200);
            DrawLines();
            emit CurrentRangeChanged(Range->parameters);
        }
    }
    else if (SV==svUpperZero)
    {
        if ((P.x()-1>=Range->parameters.UpperTop) && (P.x()-1<128))
        {
            Range->parameters.UpperZero=P.x()-1;
            DrawLines();
            emit CurrentRangeChanged(Range->parameters);
        }
        else
        {
            Range->parameters.UpperZero=P.x()-1;
            P.setY(Range->parameters.Volume);
            PutPoint(Range,P,svUpper);
        }
    }
    else if (SV==svLowerZero)
    {
        if ((P.x()<=Range->parameters.LowerTop) && (P.x()>=0))
        {
            Range->parameters.LowerZero=P.x();
            DrawLines();
            emit CurrentRangeChanged(Range->parameters);
        }
        else
        {
            Range->parameters.LowerZero=P.x();
            P.setY(Range->parameters.Volume);
            PutPoint(Range,P,svLower);
        }
    }
}


bool inline InsidePoint(int X,int Y,QPoint P)
{
    QRect r(X-6,Y-6,12,12);
    return r.contains(P);
}


void CKeyRangesControl::Draw()
{    
    DrawKeys();
    DrawLines();
}

void CKeyRangesControl::DrawLines()
{
    if (m_Sampler==nullptr) return;
    canvasLayers[0]->clearTransparent();
    for (int i=0;i<m_Sampler->rangeCount();i++)
    {
        CSampleKeyRange::RangeParams RP=m_Sampler->RangeParams(m_Sampler->currentLayerIndex,i);
        int GrafVol=Vol2Graph(RP.Volume);
        (m_Sampler->currentRangeIndex==i) ? canvasLayers[0]->setPen(QPen(Qt::red)) : canvasLayers[0]->setPen(QPen(Qt::black));
        canvasLayers[0]->drawLine(X2Graph(RP.LowerZero),Vol2Graph(0),X2Graph(RP.LowerTop),GrafVol);
        canvasLayers[0]->drawLine(X2Graph(RP.LowerTop),GrafVol,X2Graph(RP.UpperTop+1),GrafVol);
        canvasLayers[0]->drawLine(X2Graph(RP.UpperTop+1),GrafVol,X2Graph(RP.UpperZero+1),Vol2Graph(0));
        if (m_Sampler->currentRangeIndex==i)
        {
            canvasLayers[0]->setBrush(QBrush(Qt::NoBrush));
            canvasLayers[0]->drawCircle(X2Graph(RP.LowerZero),Vol2Graph(0),3);
            canvasLayers[0]->drawCircle(X2Graph(RP.LowerTop),GrafVol,3);
            canvasLayers[0]->drawCircle(X2Graph(RP.UpperTop+1),GrafVol,3);
            canvasLayers[0]->drawCircle(X2Graph(RP.UpperZero+1),Vol2Graph(0),3);
        }
    }
    update();
}

int inline CKeyRangesControl::Vol2Graph(int Vol)
{
    return (((200-Vol)*(height()-(Range_FrameWidth*2)))/200)+Range_FrameWidth;
}

int inline CKeyRangesControl::Graph2Vol(int Y)
{
    return 200-(((Y-Range_FrameWidth)*200)/(height()-(Range_FrameWidth*2)));
}

int inline CKeyRangesControl::X2Graph(int X)
{
    return ((float(X)*float(width()-(Range_FrameWidth*2)))/128.f)+Range_FrameWidth;
}

int inline CKeyRangesControl::Graph2X(int X)
{
    return (float(X-Range_FrameWidth)*128.f)/(float(width()-(Range_FrameWidth*2)));
}

void CKeyRangesControl::DrawKeys()
{
    clearGradient();
    setPen(QPen(Qt::darkGray));
    for (int i=0;i<128;i++)
    {
        const int XG=X2Graph(i);
        drawLine(XG,0,XG,height()-1);
        const int Key=i % 12;
        const int KeyHeight=(height()*3)/5;
        if (Key==1 || Key==3 || Key==6 || Key==8 || Key==10)
        {
            drawLine(XG+1,KeyHeight-5,XG+1,KeyHeight);
            for (int i1=2;i1<5;i1++)
            {
                drawLine(XG+i1,0,XG+i1,KeyHeight);
            }
        }
    }
}

void CKeyRangesControl::mousePressEvent(QMouseEvent *event)
{
    OldPoint=event->pos();
    MD=true;
    if (event->modifiers() & Qt::SHIFT)
    {
        StartMark=Graph2X(OldPoint.x());
    }
}

void CKeyRangesControl::mouseMoveEvent(QMouseEvent *event)
{
    if (m_Sampler->rangeCount())
    {
        QPoint P=event->pos();
        if (MD)
        {
            if (P != OldPoint)
            {
                MoveLines(P);
                OldPoint=P;
            }
        }
        else
        {
            CSampleKeyRange::RangeParams RP=m_Sampler->RangeParams();
            if (InsidePoint(X2Graph(RP.UpperTop+1),Vol2Graph(RP.Volume),P))
            {
                setCursor(Qt::SizeAllCursor);
                SplitValue=svUpper;
            }
            else if (InsidePoint(X2Graph(RP.LowerTop),Vol2Graph(RP.Volume),P))
            {
                setCursor(Qt::SizeAllCursor);
                SplitValue=svLower;
            }
            else if (InsidePoint(X2Graph(RP.LowerZero),Vol2Graph(0),P))
            {
                setCursor(Qt::SizeHorCursor);
                SplitValue=svLowerZero;
            }
            else if (InsidePoint(X2Graph(RP.UpperZero+1),Vol2Graph(0),P))
            {
                setCursor(Qt::SizeHorCursor);
                SplitValue=svUpperZero;
            }
            else
            {
                setCursor(Qt::ArrowCursor);
                SplitValue=svNone;
            }
        }
    }
}

void CKeyRangesControl::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint P=event->pos();
    if (event->modifiers() & Qt::SHIFT)
    {
        if (MD)
        {
            if (StartMark < Graph2X(P.x())) emit AddRangeRequested(Graph2X(P.x()),StartMark);
            if (StartMark > Graph2X(P.x())) emit AddRangeRequested(StartMark,Graph2X(P.x()));
            emit WaveFileRequested();
        }
    }
    else
    {
        if (MD)
        {
            if (SplitValue==svNone)
            {
                for (int i=0;i<m_Sampler->rangeCount();i++)
                {
                    CSampleKeyRange::RangeParams RP=m_Sampler->RangeParams(m_Sampler->currentLayerIndex,i);
                    if ((P.x()>(X2Graph(RP.LowerTop))) && (P.x()<X2Graph(RP.UpperTop+1)))
                    {
                        if (i != m_Sampler->currentRangeIndex)
                        {
                            emit RangeIndexChanged(i);
                            MD=false;
                            return;
                        }
                    }
                }
            }
            else
            {
                MoveLines(P);
            }
        }
    }
    MD=false;
}
