#ifndef CPEAKCONTROL_H
#define CPEAKCONTROL_H

#include "qcanvas.h"

namespace Ui {
    class CPeakControl;
}

class CPeakControl : public QCanvas
{
    Q_OBJECT

public:
    explicit CPeakControl(QWidget *parent = nullptr);
    ~CPeakControl();
    void setValue(const float Value);
    void reset();
    void setMargin(int margin);
    void setMax(int max);
protected:
    void resizeEvent(QResizeEvent *event);
private:
    enum LEDColors
    {
        LEDBlack,
        LEDGreen,
        LEDYellow,
        LEDRed
    };
    Ui::CPeakControl *ui;
    float m_Value;
    float m_Max;
    int m_MaxY;
    int m_OldY;
    int m_YellowBreak;
    int m_RedBreak;
    int m_Zero;
    void updateSize();
    QLinearGradient lgBlack;
    QLinearGradient lgRed;
    QLinearGradient lgYellow;
    QLinearGradient lgGreen;
    int inline val2y(const float val) const
    {
        return qMax<int>(m_HalfHeight-int(val*m_Zero),0)+m_HalfMargin;
    }
    const QLinearGradient inline y2col(const int y) const
    {
        if (y > m_YellowBreak) return lgGreen;
        if (y > m_RedBreak) return lgYellow;
        return lgRed;
    }
    LEDColors inline LEDColor(const int y) const
    {
        if (y > m_YellowBreak) return LEDGreen;
        if (y > m_RedBreak) return LEDYellow;
        return LEDRed;
    }
    void inline drawColorLED(const int y,QCanvasLayer* L)
    {
        L->setBrush(y2col(y));
        drawLED(y,L);
    }
    void inline drawLED(const int y,QCanvasLayer* L)
    {
        L->drawRectangle(m_Left,y*2,m_Width,1);
    }
    static const int Border=4;
    static const int HalfBorder=2;
    int m_Margin;
    int m_HalfMargin;
    int m_HalfHeight;
    int m_Left;
    int m_Right;
    int m_Width;
    int m_MaxValue;
};

#endif // CPEAKCONTROL_H
