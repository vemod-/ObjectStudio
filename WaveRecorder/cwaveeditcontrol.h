#ifndef CWAVEEDITCONTROL_H
#define CWAVEEDITCONTROL_H

#include <QMouseEvent>
#include "qgraphicsviewzoomer.h"
#include <QScrollBar>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include "cwavegenerator.h"

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
    //QRect visibleRect();
    bool Enabled;
    bool Region = false;
public slots:
    void scrollToPos(double Start);
    void scrollbarUpdate(int x);
    void scrollToSample(long64 s);
    void ZoomOut();
    void ZoomIn();
    void ZoomMin();
    void ZoomMax();
    void setZoom(double z);
    void ZoomToPoint(double z, const QPointF p);
    void ZoomToCursor(double z, double o);
    void ZoomRegion();
private slots:
    void Paint();
protected:
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void showEvent(QShowEvent* event);
signals:
    void ParameterChanged(CWaveGenerator::LoopParameters LP);
    void ZoomChanged();
private:
    enum WaveLineValues
    {wlNone,wlStart,wlEnd,wlLoopStart,wlLoopEnd,wlFadeIn,wlFadeOut,wlSustain};
    Ui::CWaveEditControl *ui;
    CWaveGenerator* m_WG;
    CWaveGenerator::LoopParameters m_LP;
    QGraphicsViewZoomer* zoomer;
    QGraphicsScene Scene;
    bool m_LoopOn;
    bool MD;
    double noScrollbarUpdate = false;
    double SampleToPos(ulong64 Sample);
    ulong64 PosToSample(double X);
    int Pos2Vol(int Pos);
    int Vol2Pos(int Vol);
    void DrawLines(CWaveGenerator::LoopParameters LP,bool LoopOn);
    QGraphicsItem* DrawWave();
    void inline DrawLine(QGraphicsLineItem* l, ulong64 Sample);
    void inline MoveWaveLines(QPointF p);
    WaveLineValues WaveLines;
    QPointF OldPos;
    ulong64 m_Length;
    float* m_Buffer;
    QGraphicsItem* DragItem = nullptr;
    QGraphicsLineItem* m_StartLine;
    QGraphicsLineItem* m_EndLine;
    QGraphicsLineItem* m_LoopStartLine;
    QGraphicsLineItem* m_LoopEndLine;
    QGraphicsLineItem* m_AttackLine;
    QGraphicsLineItem* m_SustainLine;
    QGraphicsLineItem* m_ReleaseLine;
    QGraphicsEllipseItem* m_Point1;
    QGraphicsEllipseItem* m_Point2;
    QGraphicsLineItem* m_VideoFadeInLine;
    QGraphicsLineItem* m_VideoFadeOutLine;
    QGraphicsLineItem* m_VideoOpaqueLine;
    QGraphicsEllipseItem* m_VideoPoint1;
    QGraphicsEllipseItem* m_VideoPoint2;

    QGraphicsItem* m_WaveItem = nullptr;
};

#endif // CWAVEEDITCONTROL_H
