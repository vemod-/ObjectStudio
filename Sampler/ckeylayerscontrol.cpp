#include "ckeylayerscontrol.h"
#include "ui_ckeylayerscontrol.h"

CKeyLayersControl::CKeyLayersControl(QWidget *parent) :
    QCanvas(parent),
    ui(new Ui::CKeyLayersControl)
{
    ui->setupUi(this);
    MD=false;
    setMouseTracking(true);
    m_Sampler=nullptr;
}

CKeyLayersControl::~CKeyLayersControl()
{
    delete ui;
}

void CKeyLayersControl::Init(CSamplerDevice *D)
{
    m_Sampler=D;
    Draw();
}

void CKeyLayersControl::resizeEvent(QResizeEvent* event)
{
    QCanvas::resizeEvent(event);
    Draw();
}

void inline CKeyLayersControl::MoveLines(QPoint P)
{
    if (m_Sampler->layerCount()) PutPoint(m_Sampler->currentLayer(),QPoint(Graph2Vol(P.x()),Graph2Y(P.y())),SplitValue);
}

void inline CKeyLayersControl::PutPoint(CLayer *Layer, QPoint P, SplitterValues SV)
{
    if (SV==svUpper)
    {
        if ((P.y()-1>Layer->parameters.LowerTop) && (P.y()-1<128))
        {
            Layer->parameters.UpperTop=P.y()-1;
            Layer->parameters.UpperZero = qMax<int>(P.y()-1,Layer->parameters.UpperZero);
            Layer->parameters.Volume=qBound<int>(5, P.x(), 200);
            Draw();
            emit CurrentLayerChanged(Layer->parameters);
        }
    }
    else if (SV==svLower)
    {
        if ((P.y()<Layer->parameters.UpperTop) && (P.y()>0))
        {
            Layer->parameters.LowerTop=P.y();
            Layer->parameters.LowerZero = qMin<int>(P.y(),Layer->parameters.LowerZero);

            Layer->parameters.Volume=qBound<int>(5, P.x(), 200);
            Draw();
            emit CurrentLayerChanged(Layer->parameters);
        }
    }
    else if (SV==svUpperZero)
    {
        if (P.y()-1>=Layer->parameters.UpperTop && P.y()-1<128)
        {
            Layer->parameters.UpperZero = qMin<int>(127,P.y()-1);
            Draw();
            emit CurrentLayerChanged(Layer->parameters);
        }
        else
        {
            Layer->parameters.UpperZero = qMin<int>(127,P.y()-1);
            P.setX(Layer->parameters.Volume);
            PutPoint(Layer,P,svUpper);
        }
    }
    else if (SV==svLowerZero)
    {
        if (P.y()<=Layer->parameters.LowerTop && P.y()>0)
        {
            Layer->parameters.LowerZero = qMax<int>(0,P.y()-1);
            Draw();
            emit CurrentLayerChanged(Layer->parameters);
        }
        else
        {
            Layer->parameters.LowerZero = qMax<int>(0,P.y()-1);
            P.setX(Layer->parameters.Volume);
            PutPoint(Layer,P,svLower);
        }

    }
}

bool inline InsidePoint(int X,int Y,QPoint P)
{
    return QRect(X,Y,0,0).adjusted(-4,-4,4,4).contains(P);
}


void CKeyLayersControl::Draw()
{
    clearGradient();
    if (m_Sampler==nullptr) return;
    for (int i=0;i<m_Sampler->layerCount();i++)
    {
        CLayer::LayerParams LP=m_Sampler->LayerParams(i);
        int GrafVol=Vol2Graph(LP.Volume);
        (m_Sampler->currentLayerIndex==i) ? setPen(QPen(Qt::red)) : setPen(QPen(Qt::black));
        drawLine(Vol2Graph(0),Y2Graph(LP.LowerZero),GrafVol,Y2Graph(LP.LowerTop));
        drawLine(GrafVol,Y2Graph(LP.LowerTop),GrafVol,Y2Graph(LP.UpperTop+1));
        drawLine(GrafVol,Y2Graph(LP.UpperTop+1),Vol2Graph(0),Y2Graph(LP.UpperZero+1));
        if (m_Sampler->currentLayerIndex==i)
        {
            setBrush(QBrush(Qt::NoBrush));
            drawCircle(Vol2Graph(0),Y2Graph(LP.LowerZero),3);
            drawCircle(GrafVol,Y2Graph(LP.LowerTop),3);
            drawCircle(GrafVol,Y2Graph(LP.UpperTop+1),3);
            drawCircle(Vol2Graph(0),Y2Graph(LP.UpperZero+1),3);
        }
    }
    update();
}

int inline CKeyLayersControl::Vol2Graph(int Vol)
{
    return (float(Vol)*float(width()-(Layer_FrameWidth*2))*0.005f)+Layer_FrameWidth;
}

int inline CKeyLayersControl::Graph2Vol(int X)
{
    return float(X-Layer_FrameWidth)/float(width()-(Layer_FrameWidth*2))*200.f;
}

int inline CKeyLayersControl::Y2Graph(int Y)
{
    return (((128.f-float(Y))*float(height()-(Layer_FrameWidth*2)))/128.f)+Layer_FrameWidth;
}

int inline CKeyLayersControl::Graph2Y(int Y)
{
    return 128.f-((float(Y-Layer_FrameWidth)*128.f)/float(height()-(Layer_FrameWidth*2)));
}

void CKeyLayersControl::mousePressEvent(QMouseEvent *event)
{
    OldPoint=event->pos();
    MD=true;
    if (event->modifiers() & Qt::SHIFT)
    {
        StartMark=Graph2Y(OldPoint.y());
    }
}

void CKeyLayersControl::mouseMoveEvent(QMouseEvent *event)
{
    if (m_Sampler->layerCount())
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
            CLayer::LayerParams LP=m_Sampler->LayerParams();
            if (InsidePoint(Vol2Graph(LP.Volume),Y2Graph(LP.UpperTop+1),P))
            {
                setCursor(Qt::SizeAllCursor);
                SplitValue=svUpper;
            }
            else if (InsidePoint(Vol2Graph(LP.Volume),Y2Graph(LP.LowerTop),P))
            {
                setCursor(Qt::SizeAllCursor);
                SplitValue=svLower;
            }
            else if (InsidePoint(Vol2Graph(0),Y2Graph(LP.LowerZero),P))
            {
                setCursor(Qt::SizeVerCursor);
                SplitValue=svLowerZero;
            }
            else if (InsidePoint(Vol2Graph(0),Y2Graph(LP.UpperZero+1),P))
            {
                setCursor(Qt::SizeVerCursor);
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

void CKeyLayersControl::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint P=event->pos();
    if (event->modifiers() & Qt::SHIFT)
    {
        if (MD)
        {
            (StartMark < Graph2Y(P.y())) ? emit AddLayerRequested(Graph2Y(P.y()),StartMark) : emit AddLayerRequested(StartMark,Graph2Y(P.y()));
        }
    }
    else
    {
        if (MD)
        {
            if (SplitValue==svNone)
            {
                for (int i=0;i<m_Sampler->layerCount();i++)
                {
                    CLayer::LayerParams LP=m_Sampler->LayerParams(i);
                    if ((P.y()>(Y2Graph(LP.UpperTop+1))) && (P.y()<Y2Graph(LP.LowerTop)))
                    {
                        emit LayerIndexChanged(i);
                        MD=false;
                        return;
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
