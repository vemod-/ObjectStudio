#include "cdbscale.h"
#include "ui_cdbscale.h"
#include "softsynthsdefines.h"
#include <QPainter>

CdBScale::CdBScale(QWidget *parent) :
    QCanvas(parent),
    ui(new Ui::CdBScale)
{
    ui->setupUi(this);
    m_Margin=Border;
    m_HalfMargin=HalfBorder;
    //SetSize();
}

CdBScale::~CdBScale()
{
    delete ui;
}

const int CdBScale::val2y(const float val, const float height)
{
    return height-((val*height*2)/3);
}

void CdBScale::SetSize()
{
    SetPen(QPen(Qt::NoPen));
    QLinearGradient gradient;
    gradient.setStart(0,0);
    gradient.setFinalStop(0,height());
    gradient.setColorAt(0, QColor(60,60,60));
    gradient.setColorAt(0.8, Qt::black);
    SetBrush(gradient);
    Rectangle(rect());

    SetPenBrush(Qt::gray);
    SetFont(QFont(QString(),9));
    int HalfHeight=(height()/2.0)-m_Margin;
    int Left=Border;
    int Right=width()-Border;
    int Width=Right-Left;

    static QList<int>LinVals=QList<int>() << -30 << -15 << -10 << -5 << 0 << 3;
    foreach(int i,LinVals)
    {
        float dbVal=db2lin(i);
        int val=val2y(dbVal,HalfHeight)+m_HalfMargin;
        if (val<m_HalfMargin) val=m_HalfMargin;
        SetPen(QPen(Qt::gray,0));
        Line(Left,val*2,Left+Width,val*2);
        //Rectangle(Left,val*2,Width,1);
        QFontMetrics fm(QFont(QString(),9));
        int L=(width()-fm.width(QString::number(i)))/2;
        Text(L-1,(val*2)-1,QString::number(i));
    }
    for (int i=-5;i<5;i++)
    {
        float dbVal=db2lin(i);
        int val=val2y(dbVal,HalfHeight)+m_HalfMargin;
        if (val<m_HalfMargin) break;
        //Rectangle(Left,val*2,Width,1);
        SetPen(QPen(Qt::gray,0));
        Line(Left,val*2,Left+Width,val*2);
    }
    update();
}

void CdBScale::resizeEvent(QResizeEvent* event)
{
    QCanvas::resizeEvent(event);
    SetSize();
}

void CdBScale::setMargin(int margin)
{
    int hm=margin*0.5;
    if (hm != m_HalfMargin)
    {
        m_HalfMargin=hm;
        m_Margin=hm*2;
        SetSize();
    }
}
