#ifndef CDBSCALE_H
#define CDBSCALE_H

#include "qcanvas.h"

namespace Ui {
    class CdBScale;
}

class CdBScale : public QCanvas
{
    Q_OBJECT

public:
    explicit CdBScale(QWidget *parent = nullptr);
    ~CdBScale();
    void setMargin(int margin);
    inline void setMax(int max)
    {
        m_Max=max;
        updateSize();
    }
protected:
    void resizeEvent(QResizeEvent *event);
private:
    Ui::CdBScale *ui;
    void updateSize();
    inline int val2y(const float val, const float height) const
    {
        const float f=100.f/m_Max;
        return int(height-(val*height*f));
    }
    static const int Border=4;
    static const int HalfBorder=2;
    int m_Margin;
    int m_HalfMargin;
    int m_Max;
};

#endif // CDBSCALE_H
