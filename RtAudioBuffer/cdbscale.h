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
    explicit CdBScale(QWidget *parent = 0);
    ~CdBScale();
    void setMargin(int margin);
protected:
    void resizeEvent(QResizeEvent *event);
private:
    Ui::CdBScale *ui;
    void SetSize();
    int val2y(const float val, const float height);
    static const int Border=4;
    static const int HalfBorder=2;
    int m_Margin;
    int m_HalfMargin;
};

#endif // CDBSCALE_H
