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
    void tick() override {}
    void play(const bool /*FromStart*/) override {
        m_TimerID = startTimer(40);
    }
    void pause() override {
        if (m_TimerID) killTimer(m_TimerID);
        m_TimerID = 0;
    }
    void skip(const ulong64 samples) override {
        m_TimeLine.skip(samples);
    }
protected:
    void resizeEvent(QResizeEvent*) override;
    void timerEvent(QTimerEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override
    {
        Q_UNUSED(rect);
        m_TimeLine.drawPlayLine(painter);
    }
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
