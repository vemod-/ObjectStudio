#ifndef CTIMELINESLIDER_H
#define CTIMELINESLIDER_H

#include <QWidget>
#include <QGraphicsView>
#include "ctimeline.h"
#include <QLabel>
#include <QLCDNumber>

class CLCDDisplay {
public:
    explicit CLCDDisplay(QLCDNumber* lcd) {
        LCD = lcd;
        lcd->display(mSecsToText(currentmSec,true));
    }
    void showTime(IDevice* d) {
        const ulong64 s = d->requestCurrentMilliSecond();
        if (s != currentmSec) {
            currentmSec = s;
            LCD->display(mSecsToText(s,true));
        }
    }
    ulong64 currentmSec = 0;
    QLCDNumber* LCD;
};

class CTimeLineSlider : public QGraphicsView, public ITicker
{
    Q_OBJECT
public:
    explicit CTimeLineSlider(QWidget *parent = nullptr);
    void init(IDevice* d, QLCDNumber* LCD = nullptr) {
        m_Device = d;
        if (LCD) m_Display = new CLCDDisplay(LCD);
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
        if (m_Display) m_Display->showTime(m_Device);
    }
    void skip(const ulong64 samples) override {
        m_TimeLine.skip(samples);
        if (m_Display) m_Display->showTime(m_Device);
    }
protected:
    bool event(QEvent* e) override {
        if (e->type()==QEvent::Leave) {
            InfoLabel.hide();
        }
        return QGraphicsView::event(e);
    }
    void closeEvent(QCloseEvent* e) override {
        InfoLabel.hide();
    }
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
    QLabel InfoLabel;
    int m_TimerID = 0;
    IDevice* m_Device;
    bool m_MDtimeline = false;
    CLCDDisplay* m_Display = nullptr;
signals:

};

#endif // CTIMELINESLIDER_H
