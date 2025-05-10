#ifndef CADSRCONTROL_H
#define CADSRCONTROL_H

#include "cadsr.h"
#include <QGraphicsView>

namespace ADSRControl
{
const int ADSR_MaxTime = 20000;
const int ADSR_MinWidth = 400;
}

namespace Ui {
    class CADSRControl;
}

class CADSRControl : public QGraphicsView
{
    Q_OBJECT

public:
    enum SplitterValuesADSR
    {svADSRNone,svDelay,svAttack,svHold,svDecay,svSustain,svRelease};
    explicit CADSRControl(QWidget *parent = nullptr);
    ~CADSRControl();
public slots:
    void Draw(CADSR::ADSRParams ADSRParams);
    void setDelay(int v);
    void setAttack(int v);
    void setHold(int v);
    void setDecay(int v);
    void setSustain(int v);
    void setRelease(int v);
private slots:
    void Paint();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
signals:
    void Changed(CADSR::ADSRParams ADSRParams);
    void DelayChanged(int v);
    void AttackChanged(int v);
    void HoldChanged(int v);
    void DecayChanged(int v);
    void SustainChanged(int v);
    void ReleaseChanged(int v);
private:
    Ui::CADSRControl *ui;
    int inline Time2X(const ldouble Time);
    int inline Vol2Y(const int Vol);
    ldouble inline X2Time(const int X);
    int inline Y2Vol(const int Y);
    void inline MoveLines(QPoint P);
    void inline PutPoint(const QPoint P,const SplitterValuesADSR SV);
    bool MD;
    QPoint OldPoint;
    SplitterValuesADSR SplitValue;
    CADSR ADSR;
    ulong64 MaxWidth;
    ulong64 ReleaseStart;
    QGraphicsScene Scene;
    QPoint from;
    QPen Pen = QPen(Qt::black);
    void moveTo(int x, int y) { from = QPoint(x,y); }
    void lineTo(int x, int y) { QPoint to(x,y);
        Scene.addLine(from.x(),from.y(),x,y,Pen);
        from = to;
    }
    void drawCircle(int x, int y, int r) {
        Scene.addEllipse(QRect(x-r,y-r,r*2,r*2),Pen);
    }
};

#endif // CADSRCONTROL_H
