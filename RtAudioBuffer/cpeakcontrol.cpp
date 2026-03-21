#include "cpeakcontrol.h"
#include "ui_cpeakcontrol.h"

CPeakControl::CPeakControl(QWidget *parent) :
    QCanvas(parent,1),
    ui(new Ui::CPeakControl)
{
    ui->setupUi(this);
    m_MaxValue = 150;
    m_Value = 0;
    m_Margin = Border;
    updateSize();
}

void CPeakControl::reset()
{
    m_Max = 0;
    m_MaxY = val2y(m_Max);
    m_Value = 0;
    m_OldY = val2y(0);

    QLinearGradient gradient;
    gradient.setStart(0,0);
    gradient.setFinalStop(0,height());
    gradient.setColorAt(0, QColor(60,60,60));
    gradient.setColorAt(0.8, Qt::black);
    fillRect(rect(),gradient);
    for (int i = val2y(1.5); i <= val2y(0); i++) drawPixmap(QPoint(m_Left, scaleY(i)),pixBlack);
    setPen(Qt::darkGray);
    setBrush(QBrush(Qt::NoBrush));
    drawRectangle(HalfBorder,m_Margin - HalfBorder,width() - Border - 1,height() - (m_Margin * 2) + Border);
    m_OverlayPix = QPixmap(size());
    QPainter p(&m_OverlayPix);
    p.fillRect(rect(),gradient);
    for (int i = val2y(1.5); i <= val2y(0); i++) p.drawPixmap(QPoint(m_Left, scaleY(i)),y2colPix(i));
    canvasLayers[0]->clearTransparent();
    update();
}

void CPeakControl::updateSize()
{
    m_LEDHeight = qMax(3,(height() - (m_Margin * 2)) / 150);
    m_ScaleHeight = qRound((height() - (m_Margin * 2)) / (double)m_LEDHeight);
    m_Left = Border;
    m_Width = width() - (Border * 2);
    m_Zero = (m_ScaleHeight * 100.0) / m_MaxValue;
    m_YellowBreak = val2y(1.f);
    m_RedBreak = val2y(1.12f);

    QLinearGradient lgBlack;
    QLinearGradient lgRed;
    QLinearGradient lgYellow;
    QLinearGradient lgGreen;

    lgBlack.setStart(0,0);
    lgBlack.setFinalStop(m_Width,0);
    lgBlack.setColorAt(0,QColor(0x66,0x66,0x66));
    lgBlack.setColorAt(0.5,QColor(0x33,0x33,0x33));
    lgBlack.setColorAt(0.9,QColor(0x22,0x22,0x22));
    lgBlack.setColorAt(1,Qt::black);

    pixBlack = LEDPix(lgBlack);

    lgYellow.setStart(0,0);
    lgYellow.setFinalStop(m_Width,0);
    lgYellow.setColorAt(0,Qt::white);
    lgYellow.setColorAt(0.3,Qt::yellow);
    lgYellow.setColorAt(1,Qt::darkYellow);

    pixYellow = LEDPix(lgYellow);

    lgRed.setStart(0,0);
    lgRed.setFinalStop(m_Width,0);
    lgRed.setColorAt(0,Qt::white);
    lgRed.setColorAt(0.3,Qt::red);
    lgRed.setColorAt(1,Qt::darkRed);

    pixRed = LEDPix(lgRed);

    lgGreen.setStart(0,0);
    lgGreen.setFinalStop(m_Width,0);
    lgGreen.setColorAt(0,Qt::white);
    lgGreen.setColorAt(0.3,Qt::green);
    lgGreen.setColorAt(1,Qt::darkGreen);

    pixGreen = LEDPix(lgGreen);

    reset();
}

CPeakControl::~CPeakControl()
{
    delete ui;
}

void CPeakControl::setValue(const float Value)
{
    QCanvasLayer* L = canvasLayers[0];
    if (Value > m_Value) {
        m_Value = Value;
        m_Max = fmaxf(m_Value,m_Max);
        m_MaxY = val2y(m_Max);
    }
    else {
        m_Value = (m_Value > 0.00005f) ? qMin<float>(m_Value * 0.96f,2) : 0;
    }
    const int y = val2y(m_Value);
    if (y == m_OldY) return;
    if (m_OldY > y) {
        const int top = scaleY(y);
        const int bottom = scaleY(m_OldY);
        QRect r(m_Left,top,m_Width,(bottom - top) + m_LEDHeight);
        L->drawPixmap(r.topLeft(),m_OverlayPix,r);
        update(r);
    }
    else if (m_MaxY < y)
    {
        int top = scaleY(qMax(m_OldY,m_MaxY + 1));
        const int bottom = scaleY(y);
        QRect r(QRect(m_Left,top,m_Width,(bottom - top) + m_LEDHeight));
        L->eraseTransparent(r);
        update(r);
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
    m_Margin = margin;
    updateSize();
}

void CPeakControl::setMax(int max)
{
    m_MaxValue = max;
    updateSize();
}
