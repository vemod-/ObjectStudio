#ifndef CTIMELINESLIDER_H
#define CTIMELINESLIDER_H

#include <QWidget>
#include <QGraphicsView>
#include "ctimeline.h"
#include <QLabel>

class CTimeLineSlider : public QGraphicsView, public ITicker
{
    Q_OBJECT
public:
    explicit CTimeLineSlider(QWidget *parent = nullptr);
    void init(IDevice* d) {
        m_Device = d;
        d->addTickerDevice(this);
        draw();
    }
    void draw() {
        m_TimeLine.setSamples(m_Device->requestSamples());
        Scene.clear();
        m_TimeLine.render(&Scene);
    }
    void tick() {}
    void play(const bool /*FromStart*/) {
        m_TimerID = startTimer(40);
    }
    void pause() {
        if (m_TimerID) killTimer(m_TimerID);
        m_TimerID = 0;
    }
    void skip(const ulong64 samples) {
        m_TimeLine.skip(samples);
    }
protected:
    void resizeEvent(QResizeEvent*);
    void timerEvent(QTimerEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
private:
    QGraphicsScene Scene;
    CTimeLine m_TimeLine;
    QLabel* InfoLabel;
    int m_TimerID = 0;
    IDevice* m_Device;
    bool m_MDtimeline = false;
signals:

};

#endif // CTIMELINESLIDER_H
