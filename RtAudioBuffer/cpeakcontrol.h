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
    int m_LEDHeight = 3;
    void updateSize();
    QPixmap pixBlack;
    QPixmap pixRed;
    QPixmap pixYellow;
    QPixmap pixGreen;
    int inline scaleY(const int val) const {
         return (val * m_LEDHeight) + (m_Margin - m_LEDHeight);
    }
    int inline val2y(const float val) const {
        return qMax<int>(m_ScaleHeight - int(val * m_Zero),0);
    }
    const inline QPixmap& y2colPix(const int y) const
    {
        if (y > m_YellowBreak) return pixGreen;
        if (y > m_RedBreak) return pixYellow;
        return pixRed;
    }
    QPixmap LEDPix(QLinearGradient& lg) const {
        QPixmap pix = QPixmap(m_Width,m_LEDHeight - 1);
        QPainter p(&pix);
        p.fillRect(0,0,m_Width,m_LEDHeight - 1,lg);
        return pix;
    }
    static const int Border = 4;
    static const int HalfBorder = 2;
    int m_Margin;
    int m_ScaleHeight;
    int m_Left;
    int m_Width;
    int m_MaxValue;
    QPixmap brushPix;
    QPixmap m_OverlayPix;
};

#endif // CPEAKCONTROL_H
