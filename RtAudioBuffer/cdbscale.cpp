#include "cdbscale.h"
#include "ui_cdbscale.h"
#include "softsynthsdefines.h"
#include <QPainter>
#include <array>

CdBScale::CdBScale(QWidget *parent) :
    QCanvas(parent),
    ui(new Ui::CdBScale)
{
    ui->setupUi(this);
    m_Margin=Border;
    m_HalfMargin=HalfBorder;
    m_Max=150;
}

CdBScale::~CdBScale()
{
    delete ui;
}

void CdBScale::updateSize()
{
    static const int LinVals[8]={-30, -15, -10, -5, 0, 3, 5, 6};
    setPen(QPen(Qt::NoPen));
    QLinearGradient gradient;
    gradient.setStart(0,0);
    gradient.setFinalStop(0,height());
    gradient.setColorAt(0, QColor(60,60,60));
    gradient.setColorAt(0.8, Qt::black);
    setBrush(gradient);
    drawRectangle(rect());

    setPenBrush(Qt::gray);
    setLayerFontSize(9);
    const int HalfHeight=(height()/2)-m_Margin;
    const int Left=Border;
    const int Right=width()-Border;
    const int Width=Right-Left;

    setPen(QPen(Qt::gray,0));
    for (const int j : LinVals)//for(uint i=0;i<LinVals.size();i++)
    {
        //const int j=v;
        const float dbVal=dB2linf(j);
        const QString s=QString::number(j);
        int val=val2y(dbVal,HalfHeight)+m_HalfMargin;
        if (val<m_HalfMargin) break;
        drawLine(Left,val*2,Left+Width,val*2);
        QFontMetrics fm(layerFont());
        const int L=(width()-fm.horizontalAdvance(s))/2;
        drawText(L-1,(val*2)-1,s);
    }
    for (int i=-5;i<5;i++)
    {
        const float dbVal=dB2linf(i);
        const int val=val2y(dbVal,HalfHeight)+m_HalfMargin;
        if (val<m_HalfMargin) break;
        drawLine(Left,val*2,Left+Width,val*2);
    }
    update();
}

void CdBScale::resizeEvent(QResizeEvent* event)
{
    QCanvas::resizeEvent(event);
    updateSize();
}

void CdBScale::setMargin(int margin)
{
    const int hm=margin/2;
    if (hm != m_HalfMargin)
    {
        m_HalfMargin=hm;
        m_Margin=hm*2;
        updateSize();
    }
}
