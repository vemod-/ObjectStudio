#ifndef CAUTOMATIONLANE_H
#define CAUTOMATIONLANE_H

//#include <QGraphicsView>
//#include <QGraphicsItem>
#include <QLabel>
#include "../../QGraphicsViewZoomer/qgraphicsviewzoomer.h"
#include <QScrollBar>
#include <qsignalmenu.h>
#include "cdevicelist.h"
#include <QApplication>
#include "ctimeline.h"
#include "ctimelineedit.h"

namespace Ui {
class CAutomationLane;
}

class CAutomationLane : public QGraphicsView, ITicker
{
    Q_OBJECT
public:
    explicit CAutomationLane(QWidget *parent = nullptr);
    ~CAutomationLane();
    QRect visibleRect() {
        QRect r(horizontalScrollBar()->value(),verticalScrollBar()->value(),viewport()->width(),viewport()->height());
        return r;
    }
    void fill(IDevice* d, int i, CDeviceList* dl, bool timelineVisible = true) {
        m_TimeLineVisible = timelineVisible;
        if (!timelineVisible) {
            m_TimeLineHeight = 0;
            zoomer->setMax(zoomer->getZoom());
            zoomer->setMin(zoomer->getZoom());
        }
        m_TimeLine.setHeight(m_TimeLineHeight);
        m_Device = d;
        m_DL = dl;
        if (m_Device->requestIsPlaying()) m_TimerID = startTimer(50);
        d->addTickerDevice(this);
        m_TimeLine.setSamples(m_Device->requestSamples());
        m_ParameterIndex = i;
        m_ParameterMenu->clear();
        for (int i = 0; i < d->parameterCount(); i++) {
            m_ParameterMenu->addAction(d->parameter(i)->Name,i);
        }
        m_ParameterMenu->addSeparator();
        m_ParameterMenu->addAction("Close",-1);
        m_ParameterMenu->checkAction(m_ParameterIndex);
        connect(m_ParameterMenu,qOverload<int>(&QSignalMenu::menuClicked),this,&CAutomationLane::selectParameter);
        Paint();
    }
    void Paint() {
        DrawAutomation(false);
    }
    void DrawAutomation(bool TopLayer, QPointF delta = QPointF()) {
        if (TopLayer)
        {
            for (QGraphicsItem* i : std::as_const(m_TopLayer)) Scene.removeItem(i);
            m_TopLayer.clear();
        }
        else
        {
            Scene.clear();
            m_TopLayer.clear();
            QRectF r = viewport()->rect();
            r.setWidth(double(r.width())*zoomer->getZoom());
            setSceneRect(r);
            if (deviceValid())
            {
                QFont f = font();
                f.setBold(true);
                f.setPointSizeF(16);
                QGraphicsTextItem* i = Scene.addText(m_Device->deviceID() + " " + currentParameter()->Name);
                i->setPos(0,m_TimeLineHeight);
            }
            m_TimeLine.setSamples(m_Device->requestSamples());
            m_TimeLine.render(&Scene,visibleRect());
        }
        if (!deviceValid()) return;
        if (m_Marking)
        {
            m_TopLayer.append(Scene.addRect(m_Marked,QPen(Qt::lightGray),QBrush(Qt::blue)));
        }
        if (!TopLayer)
        {
            for (int i = 0; i < m_Device->parameterCount(); i++) {
                if (i != m_ParameterIndex)
                {
                    DrawLayer(m_Device->parameter(i),false);
                }
            }
        }
        m_TopLayer.append(DrawLayer(currentParameter(), true, delta));
        verticalScrollBar()->setEnabled(false);
    }
    QList<QGraphicsItem*> DrawLayer(CParameter* parameter, bool isTopLayer, QPointF delta = QPointF())
    {
        qDebug() << parameter->Name;
        QList<QGraphicsItem*> retval;
        CParameterEventList& l = parameter->events;
        QColor noEdit = Qt::red;
        QColor lines = Qt::yellow;
        QColor marked = Qt::white;
        if (!isTopLayer)
        {
            noEdit = Qt::darkRed;
            lines = Qt::darkYellow;
            marked = Qt::darkYellow;
        }
        if (l.empty())
        {
            QPoint p(translateEvent(m_TimeLine.milliSeconds(),parameter->Value,parameter));
            retval.append(Scene.addLine(0,p.y(),p.x(),p.y(),QPen(noEdit)));
            qDebug() << p;
        }
        else
        {
            if ((parameter->Type == CParameter::dB) || (parameter->Type == CParameter::Numeric) || (parameter->Type == CParameter::Percent))
            {
                QPointF oldPoint;
                for (uint i = 0; i < l.size(); i++) {
                    const CParameterEvent& e = l[i];
                    QColor c = lines;
                    QColor lc = lines;
                    QPointF p(translateEvent(e.time,e.value,parameter));
                    if (m_Selected.contains(i))
                    {
                        c = marked;
                        if (m_Selected.contains(i-1)) lc = Qt::white;
                        p += delta;
                    }
                    //qDebug() << p << e.time << e.value;
                    QRectF r(p-QPointF(2,2),p+QPointF(2,2));
                    retval.append(Scene.addEllipse(r,QPen(c)));
                    if (oldPoint != QPointF()) retval.append(Scene.addLine(oldPoint.x(),oldPoint.y(),p.x(),p.y(),QPen(lc)));
                    oldPoint = p;
                }
                retval.append(Scene.addLine(oldPoint.x(),oldPoint.y(),sceneRect().width(),oldPoint.y(),QPen(lines)));
            }
            else
            {
                QPointF oldPoint;
                for (uint i = 0; i < l.size(); i++) {
                    const CParameterEvent& e = l[i];
                    QPointF p(translateEvent(e.time,e.value,parameter));
                    if (m_Selected.contains(i))
                    {
                        p += delta;
                    }
                    //qDebug() << p << e.time << e.value;
                    if (oldPoint != QPointF()) retval.append(Scene.addLine(oldPoint.x(),oldPoint.y(),p.x(),oldPoint.y(),QPen(lines)));
                    oldPoint = p;
                }
                retval.append(Scene.addLine(oldPoint.x(),oldPoint.y(),sceneRect().width(),oldPoint.y(),QPen(lines)));
            }
        }
        return retval;
    }
    inline QString deviceId() {
        if (m_Device) return m_Device->deviceID();
        return QString();
    }
    /*
    void tick() {
        m_mSecCounter.eatTick();
    }
*/
    void play(const bool /*FromStart*/) {
        m_TimerID = startTimer(50);
        //if (FromStart) m_mSecCounter.reset();
    }
    void pause() {
        if (m_TimerID) killTimer(m_TimerID);
        m_TimerID = 0;
    }
    void skip(const ulong64 samples) {
        m_TimeLine.skip(samples);
    }
    void serialize(QDomLiteElement* xml) const {
        xml->setAttribute("DeviceID",m_Device->deviceID());
        xml->setAttribute("ParameterIndex",m_ParameterIndex);
    }
protected:
    void timerEvent(QTimerEvent* /*e*/) {
        if (!m_TimerID) return;
        //qDebug() << m_Device->requestCurrentMilliSecond();
        m_TimeLine.handleTimer(m_Device);
        //Scene.update();
    }
    void showEvent(QShowEvent* e) {
        QGraphicsView::showEvent(e);
        m_TimeLine.setCurrentSample(m_Device->requestCurrentSample());
        Paint();
    }
    void mouseDoubleClickEvent(QMouseEvent *event)
    {
        QPointF p = QGraphicsView::mapToScene(event->pos());
        if (m_TimeLine.handleDoubleClick(p,m_Device)) return;
        if (valueFromY(p.y(),currentParameter()) > currentParameter()->Max) p.setY(valueToY(currentParameter()->Max,currentParameter()));
        if (valueFromY(p.y(),currentParameter()) < currentParameter()->Min) p.setY(valueToY(currentParameter()->Min,currentParameter()));
        const int i = insideEvent(p);
        if (i > -1) {
            currentParameter()->removeEvent(i);
        }
        else
        {
            CParameterEventList& l = currentParameter()->events;
            if (l.empty())
            {
                currentParameter()->appendEvent(0,currentParameter()->Value,deviceId() + " " + currentParameter()->Name);
                currentParameter()->appendEvent(timeFromX(p.x()),valueFromY(p.y(),currentParameter()),deviceId() + " " + currentParameter()->Name);
            }
            else
            {
                currentParameter()->appendEvent(timeFromX(p.x()),valueFromY(p.y(),currentParameter()),deviceId() + " " + currentParameter()->Name);
            }
            const int i = insideEvent(p);
            if (i > -1) {
                m_Selected.clear();
                m_Selected.append(i);
            }
        }
        if (m_Device->isPlaying()) m_DL->updateAutomationPlayer();
        DrawAutomation(true);
        QGraphicsView::mouseDoubleClickEvent(event);
    }
    void mousePressEvent(QMouseEvent *event) {
        QPointF p = QGraphicsView::mapToScene(event->pos());
        if (event->button() == Qt::RightButton) {
            if (p.y() < m_TimeLineHeight) {
                CTimeLineMenu* d = new CTimeLineMenu(&m_TimeLine,this);
                connect(d,&CTimeLineMenu::Changed,this,&CAutomationLane::Paint);
                d->popup(cursor().pos());
                QGraphicsView::mousePressEvent(event);
                return;
            }
            m_ParameterMenu->popup(event->globalPosition().toPoint());
            QGraphicsView::mousePressEvent(event);
            return;
        }
        if (m_TimeLine.handleMousePress(p,m_Device)) {;
            QGraphicsView::mousePressEvent(event);
            return;
        }
        if (valueFromY(p.y(),currentParameter()) > currentParameter()->Max) p.setY(valueToY(currentParameter()->Max,currentParameter()));
        if (valueFromY(p.y(),currentParameter()) < currentParameter()->Min) p.setY(valueToY(currentParameter()->Min,currentParameter()));
        m_Orig = p;
        m_MD=true;
        m_MousePlacing = MouseOutside;
        int i = insideEvent(p);
        if (i > -1) {
            m_MousePlacing = MouseOnEvent;
        }
        else {
            i = betweenEvents(p);
            if (i > -1) m_MousePlacing = MouseOnLine;
        }
        if (m_MousePlacing == MouseOnEvent) {
            if (event->modifiers() != Qt::ControlModifier) m_Selected.clear();
            if (!m_Selected.contains(uint(i))) m_Selected.append(i);
            DrawAutomation(true);
            return;
        }
        if (m_MousePlacing == MouseOnLine)
        {
            if (event->modifiers() != Qt::ControlModifier) m_Selected.clear();
            if (!m_Selected.contains(i)) m_Selected.append(i);
            if (!m_Selected.contains(i+1)) m_Selected.append(i+1);
            DrawAutomation(true);
        }
        QGraphicsView::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event) {
        QPointF p = QGraphicsView::mapToScene(event->pos());
        if (m_TimeLine.handleMouseMove(p, m_Device)) return;
        if (valueFromY(p.y(),currentParameter()) > currentParameter()->Max) p.setY(valueToY(currentParameter()->Max,currentParameter()));
        if (valueFromY(p.y(),currentParameter()) < currentParameter()->Min) p.setY(valueToY(currentParameter()->Min,currentParameter()));
        QPointF delta = p - m_Orig;
        QPointF textPoint = p;
        if (m_MD)
        {
            if (m_MousePlacing == MouseOutside)
            {
                if (p != m_Orig)
                {
                    m_Marked = QRectF(m_Orig.x(),0,delta.x(),visibleRect().height()).normalized();
                    m_Marking = true;
                    DrawAutomation(true);
                }
            }
            else
            {
                if (p != m_Orig) DrawAutomation(true, delta);
            }
        }
        if (!m_MD)
        {
            const int i = insideEvent(p);
            if (i > -1) {
                const CParameterEvent& e = currentParameter()->events[i];
                textPoint = translateEvent(e.time,e.value,currentParameter());
            }
        }
        QString v=currentParameter()->valueText(valueFromY(textPoint.y(),currentParameter()));
        ulong64 t = timeFromX(textPoint.x());
        if (!InfoLabel->isVisible()) InfoLabel->show();
        InfoLabel->setText(v + "\n" + m_TimeLine.timeToText(t));
        InfoLabel->move(event->pos()+geometry().topLeft()+QPoint(4,4));
        //InfoLabel->setFixedSize(InfoLabel->sizeHint());
        InfoLabel->adjustSize();
        QGraphicsView::mouseMoveEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event) {
        QPointF p = QGraphicsView::mapToScene(event->pos());
        if (m_TimeLine.handleMouseRelease(p, m_Device)) return;
        if (valueFromY(p.y(),currentParameter()) > currentParameter()->Max) p.setY(valueToY(currentParameter()->Max,currentParameter()));
        if (valueFromY(p.y(),currentParameter()) < currentParameter()->Min) p.setY(valueToY(currentParameter()->Min,currentParameter()));
        QPointF delta = p - m_Orig;
        if (m_Marking)
        {
            m_Selected.clear();
            m_Marked = QRectF(m_Orig.x(),0,delta.x(),visibleRect().height()).normalized();
            CParameterEventList& l = currentParameter()->events;
            for (uint i = 0; i < l.size(); i++) {
                const CParameterEvent& e = l[i];
                if (m_Marked.contains(translateEvent(e.time,e.value,currentParameter()))) m_Selected.append(i);
            }
            if (m_Device->isPlaying()) m_DL->updateAutomationPlayer();
            m_Marked = QRectF();
            m_Marking = false;
            DrawAutomation(true);
        }
        else if (m_MD)
        {
            if (p != m_Orig)
            {
                CParameterEventList& l = currentParameter()->events;
                for (uint i = 0; i < l.size(); i++) {
                    const CParameterEvent& e = l[i];
                    if (m_Selected.contains(i))
                    {
                        QPointF o = translateEvent(e.time,e.value,currentParameter()) + delta;
                        if (i == 0) {
                            currentParameter()->changeEvent(i,0,valueFromY(o.y(),currentParameter()));
                        }
                        else {
                            currentParameter()->changeEvent(i,timeFromX(o.x()),valueFromY(o.y(),currentParameter()));
                        }
                    }
                }
                if (m_Device->isPlaying()) m_DL->updateAutomationPlayer();
                currentParameter()->sortEvents(l);
                DrawAutomation(true);
            }
        }
        m_MD = false;
        m_Marking = false;
        QGraphicsView::mouseReleaseEvent(event);
    }
    void keyPressEvent(QKeyEvent* e)
    {
        if (e->key() == Qt::Key_Backspace) {
            if (m_Selected.size())
            {
                CParameterEventList& l = currentParameter()->events;
                for (int i = m_Selected.size()-1; i >= 0; i--) l.erase(l.cbegin() + m_Selected[i]);
                if (m_Device->isPlaying()) m_DL->updateAutomationPlayer();
                currentParameter()->sortEvents(l);
                DrawAutomation(true);
            }
        }
        QGraphicsView::keyPressEvent(e);
    }
    bool event(QEvent *event) {
        if (event->type()==QEvent::Leave) InfoLabel->hide();
        return QGraphicsView::event(event);
    }
    void resizeEvent(QResizeEvent *event) {
        QGraphicsView::resizeEvent(event);
        if (m_Device)
        {
            m_TimeLine.setWidth(viewport()->width());
            Paint();
        }
    }
private slots:
    void setZoom(double zoom) {
        double s = double(horizontalScrollBar()->maximum())/horizontalScrollBar()->value();
        m_TimeLine.setZoom(zoom);
        Paint();
        horizontalScrollBar()->setValue(horizontalScrollBar()->maximum()/s);
    }
    void selectParameter(int i) {
        if (i < 0) {
            m_Device->removeTickerDevice(this);
            this->deleteLater();
            return;
        }
        m_ParameterIndex = i;
        Paint();
        m_ParameterMenu->checkAction(i);
    }
private:
    Ui::CAutomationLane *ui;
    enum mousePlacings {
        MouseOutside,
        MouseOnEvent,
        MouseOnLine
    };
    mousePlacings m_MousePlacing = MouseOutside;
    CTimeLine m_TimeLine;
    int m_ParameterIndex = 0;
    IDevice* m_Device = nullptr;
    inline bool deviceValid() {
        if (!m_Device) return false;
        if (m_Device->parameterCount() == 0) return false;
        return true;
    }
    inline CParameter* currentParameter() {
        if (deviceValid()) return m_Device->parameter(m_ParameterIndex);
        return nullptr;
    }
    QPoint translateEvent(ulong64 time, int value, CParameter* p) {
        const int y = valueToY(value,p);
        const int x = timeToX(time);
        return QPoint(x,y);
    }
    QRect eventRect(ulong64 time, int value, CParameter* p) {
        const QPoint o = translateEvent(time,value,p);
        return QRect(o - QPoint(2,2),o + QPoint(2,2));
    }
    int timeToX(ulong64 time) {
        //return (ldouble(double(width())*zoomer->getZoom()) / ldouble(m_Duration)) * ldouble(time);
        return m_TimeLine.timeToX(time);
    }
    int valueToY(int value, CParameter* p) {
        const double h = sceneRect().height() - m_TimeLineHeight;
        return m_TimeLineHeight + h - ((h / double(p->Max - p->Min)) * (double(value)-p->Min));
    }
    ulong64 timeFromX(int x) {
        //return (ldouble(m_Duration) / ldouble(double(width())*zoomer->getZoom())) * ldouble(x);
        return m_TimeLine.timeFromX(x);
    }
    int valueFromY(int y, CParameter* p) {
        const double h = sceneRect().height() - m_TimeLineHeight;
        int v = (double(p->Max - p->Min) / h * double(h-(y-m_TimeLineHeight))) + p->Min;
        //v = qMax<int>(p->Min,v);
        //v = qMin<int>(p->Max,v);
        return v;
    }
    int insideEvent(QPointF p) {
        CParameterEventList& l = currentParameter()->events;
        if (!l.empty())
        {
            for (uint i = 0; i < l.size(); i++) {
                const CParameterEvent& e = l[i];
                const QRect r1(eventRect(e.time,e.value,currentParameter()));
                if (r1.contains(p.toPoint())) return i;
            }
        }
        return -1;
    }
    int betweenEvents(QPointF p)
    {
        const ulong64 time = timeFromX(p.x());
        CParameterEventList& l = currentParameter()->events;
        if (l.size() > 1)
        {
            ulong64 t = l[0].time;
            for (uint i = 0; i < l.size()-1; i++) {
                const ulong64 t1=l[i+1].time;
                if ((time >= t) && (time <= t1))
                {
                    const QRect r1 = eventRect(time,l[i].value,currentParameter());
                    const QRect r2 = eventRect(time,l[i+1].value,currentParameter());
                    if ((r1.contains(p.toPoint())) && (r2.contains(p.toPoint()))) return i;
                }
                t = t1;
            }
        }
        return -1;
    }
    QGraphicsScene Scene;
    QList<QGraphicsItem*> m_TopLayer;
    QLabel* InfoLabel;
    bool m_MD = false;
    QPoint m_StartPoint;
    QList<uint> m_Selected;
    bool m_Marking = false;
    QRectF m_Marked;
    QPointF m_Orig;
    QGraphicsViewZoomer* zoomer;
    QSignalMenu* m_ParameterMenu;
    CDeviceList* m_DL;
    //CmSecCounter m_mSecCounter;
    int m_TimerID;
    bool m_TimeLineVisible = true;
    int m_TimeLineHeight = timelineheight;
};

#endif // CAUTOMATIONLANE_H
