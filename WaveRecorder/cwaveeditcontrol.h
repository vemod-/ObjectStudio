#ifndef CWAVEEDITCONTROL_H
#define CWAVEEDITCONTROL_H

#include <QMouseEvent>
#include "cwavegenerator.h"
//#include <QGesture>
//#include <QGraphicsView>
#include "qgraphicsviewzoomer.h"
#include <QScrollBar>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>

namespace Ui {
    class CWaveEditControl;
}

class CWaveEditControl : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CWaveEditControl(QWidget *parent = 0);
    ~CWaveEditControl();
    void Init(CWaveGenerator* WG,CWaveGenerator::LoopParameters LP,bool LoopOn);
    void Draw(CWaveGenerator::LoopParameters LP);
    QRect visibleRect() {
        return QRect(horizontalScrollBar()->value(), verticalScrollBar()->value(), viewport()->width(), viewport()->height());
    }
    //double Zoom;
    bool Enabled;
public slots:
    void scrollToPos(int Start);
    void ZoomOut();
    void ZoomIn();
    void ZoomMin();
    void ZoomMax();
    void setZoom(double z);
private slots:
    void Paint();
protected:
    //bool event(QEvent* event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void showEvent(QShowEvent* event);
    //bool gestureEvent(QGestureEvent *event);
    //void pinchTriggered(QPinchGesture *gesture);
signals:
    void ParameterChanged(CWaveGenerator::LoopParameters LP);
    void ZoomChanged();
private:
    enum WaveLineValues
    {wlNone,wlStart,wlEnd,wlLoopStart,wlLoopEnd,wlFadeIn,wlFadeOut};
    Ui::CWaveEditControl *ui;
    CWaveGenerator* m_WG;
    CWaveGenerator::LoopParameters m_LP;
    QGraphicsViewZoomer* zoomer;
    QGraphicsScene Scene;
    bool m_LoopOn;
    bool MD;
    void Scroll();
    int PicPos(ldouble Position);
    ulong64 BufferPos(int X);
    int Pos2Vol(int Pos);
    int Vol2Pos(int Vol);
    void DrawLines(CWaveGenerator::LoopParameters LP,bool LoopOn);
    void DrawWave();
    void inline DrawLine(QGraphicsLineItem* l, ulong64 Position);
    void inline MoveWaveLines(QPointF p);
    WaveLineValues WaveLines;
    QPointF OldPos;
    //bool isMinZoom;
    ulong64 m_Length;
    float* m_Buffer;
    QGraphicsLineItem* m_StartLine;
    QGraphicsLineItem* m_EndLine;
    QGraphicsLineItem* m_LoopStartLine;
    QGraphicsLineItem* m_LoopEndLine;
    QGraphicsLineItem* m_AttackLine;
    QGraphicsLineItem* m_SustainLine;
    QGraphicsLineItem* m_ReleaseLine;
    QGraphicsEllipseItem* m_Point1;
    QGraphicsEllipseItem* m_Point2;
};

#endif // CWAVEEDITCONTROL_H
