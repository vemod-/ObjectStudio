#include "cpeakcontrol.h"
#include "ui_cpeakcontrol.h"
//#include <math.h>

CPeakControl::CPeakControl(QWidget *parent) :
    QCanvas(parent,1),
    ui(new Ui::CPeakControl)
{
    ui->setupUi(this);
    m_MaxValue=150;
    m_Value=0;
    m_Margin=Border;
    m_HalfMargin=HalfBorder;
    updateSize();
}

void CPeakControl::reset()
{
    m_Max=0;
    m_MaxY=val2y(m_Max);
    m_Value=0;
    m_OldY=m_HalfHeight+m_HalfMargin;

    QLinearGradient gradient;
    gradient.setStart(0,0);
    gradient.setFinalStop(0,height());
    gradient.setColorAt(0, QColor(60,60,60));
    gradient.setColorAt(0.8, Qt::black);
    setPen(QPen(Qt::NoPen));
    setBrush(gradient);
    drawRectangle(rect());
    setPen(Qt::darkGray);
    setBrush(QBrush(Qt::NoBrush));
    drawRectangle(HalfBorder,m_Margin-HalfBorder,width()-Border-1,height()-m_Margin-m_Margin+Border);
    canvasLayers[0]->clearTransparent();
    QCanvasLayer* L=canvasLayers[0];
    L->setBrush(lgBlack);
    L->setPen(QPen(Qt::NoPen));
    for (int i=val2y(1.5);i<=val2y(0);i++) drawLED(i,L);
    update();
}

void CPeakControl::updateSize()
{
    m_HalfHeight=(height()/2)-m_Margin;
    m_Left=Border;
    m_Right=width()-Border;
    m_Width=m_Right-m_Left;
    //const double f=100.0/m_MaxValue;
    m_Zero=(m_HalfHeight*100)/m_MaxValue;
    m_YellowBreak=val2y(1.f);
    m_RedBreak=val2y(1.12f);

    lgBlack.setStart(m_Left,0);
    lgBlack.setFinalStop(m_Width,0);
    lgBlack.setColorAt(0,QColor("#666"));
    lgBlack.setColorAt(0.5,QColor("#333"));
    lgBlack.setColorAt(0.9,QColor("#222"));
    lgBlack.setColorAt(1,Qt::black);

    lgYellow.setStart(m_Left,0);
    lgYellow.setFinalStop(m_Width,0);
    lgYellow.setColorAt(0,Qt::white);
    lgYellow.setColorAt(0.3,Qt::yellow);
    lgYellow.setColorAt(1,Qt::darkYellow);

    lgRed.setStart(m_Left,0);
    lgRed.setFinalStop(m_Width,0);
    lgRed.setColorAt(0,Qt::white);
    lgRed.setColorAt(0.3,Qt::red);
    lgRed.setColorAt(1,Qt::darkRed);

    lgGreen.setStart(m_Left,0);
    lgGreen.setFinalStop(m_Width,0);
    lgGreen.setColorAt(0,Qt::white);
    lgGreen.setColorAt(0.3,Qt::green);
    lgGreen.setColorAt(1,Qt::darkGreen);

    reset();
}

CPeakControl::~CPeakControl()
{
    delete ui;
}

void CPeakControl::setValue(const float Value)
{
    QCanvasLayer* L=canvasLayers[0];
    if (Value > m_Value)
    {
        m_Value=Value;
        m_Max=fmaxf(m_Value,m_Max);
        m_MaxY=val2y(m_Max);
    }
    else
    {
        m_Value = (m_Value>0.00005f) ? qMin<float>(m_Value*0.96f,2) : 0;
    }
    const int y=val2y(m_Value);
    if (y==m_OldY) return;

    L->setPen(QPen(Qt::NoPen));
    LEDColors lc=LEDBlack;
    if (m_OldY>y)
    {
        for (int i=m_OldY;i>=y;i--)
        {
            const LEDColors currentLC = LEDColor(i);
            if (lc != currentLC)
            {
                lc=currentLC;
                drawColorLED(i,L);
            }
            else
            {
                drawLED(i,L);
            }
        }
        repaint(m_Left,y*2,m_Width,((m_OldY-y)*2)+1);
    }
    else if (m_MaxY<y)
    {
        L->setBrush(lgBlack);
        if (m_MaxY==m_OldY)
        {
            for (int i=m_OldY+1;i<=y;i++) drawLED(i,L);
        }
        else
        {
            for (int i=m_OldY;i<=y;i++) drawLED(i,L);
        }
        repaint(m_Left,m_OldY*2,m_Width,((y-m_OldY)*2)+1);
    }
    m_OldY=y;
}

void CPeakControl::resizeEvent(QResizeEvent *event)
{
    QCanvas::resizeEvent(event);
    updateSize();
}

void CPeakControl::setMargin(int margin)
{
    const int hm=margin/2;
    if (hm != m_HalfMargin)
    {
        m_HalfMargin=hm;
        m_Margin=hm*2;
        updateSize();
    }
}

void CPeakControl::setMax(int max)
{
    m_MaxValue=max;
    updateSize();
}
