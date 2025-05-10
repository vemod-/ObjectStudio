#ifndef CLANEEDITCONTROL_H
#define CLANEEDITCONTROL_H

#include <QWidget>
#include <QGraphicsView>
#include "cwavelane.h"
#include <QScrollBar>
#include "qgraphicsviewzoomer.h"
#include "ctimeline.h"
#include <QClipboard>
#include <QApplication>

namespace Ui {
class CLaneEditControl;
}

class CLaneEditControl : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CLaneEditControl(QWidget *parent = nullptr);
    ~CLaneEditControl();
    void init(CWaveLane* l, QList<int>* currentTrack, ulong64 currentSample, QDomLiteElement* timelineXML, QList<QAction*> al) {
        actionList = al;
        CurrentTrack = currentTrack;
        m_Lane = l;
        m_TimerID = startTimer(40);
        CurrentTrack = currentTrack;
        m_TimeLine.setCurrentSample(currentSample);
        m_TimeLine.unserialize(timelineXML);
        for (QAction* a : qAsConst(actionList)) {
            addAction(a);
            connect(a,&QAction::triggered,this,&CLaneEditControl::Paint,Qt::QueuedConnection);
        }
    }
    QRect visibleRect() {
        return QRect(horizontalScrollBar()->value(), verticalScrollBar()->value(), viewport()->width(), viewport()->height());
    }
    void UpdateGeometry() {
        for(int j = 0; j < m_Lane->tracks.size(); j++) m_Lane->tracks[j]->isActive = (CurrentTrack->contains(j));
        m_Lane->geometry.moveTo(0,timelineheight+8);
        m_Lane->geometry.setHeight(viewport()->height()-(timelineheight + 8));
        m_Lane->UpdateGeometry(zoomer->getZoom(),50);
    }
    void Paint() {
        Scene.clear();
        UpdateGeometry();
        QRect r = viewport()->rect();
        r.setWidth(m_Lane->geometry.width());
        setSceneRect(r);
        zoomer->setMin(zoomer->getZoom() * ldouble(viewport()->width())/ldouble(m_Lane->geometry.width()));
        m_Lane->paint(Scene,zoomer->getZoom(),visibleRect(),true);
        m_TimeLine.setFixedWidth(m_Lane->sample2Pos(m_Lane->samples()), m_Lane->samples());
        m_TimeLine.setPen(QPen(Qt::black));
        m_TimeLine.render(&Scene,visibleRect());
        verticalScrollBar()->setEnabled(false);
    }
    bool event(QEvent* e) {
        if (e->type() == QEvent::Leave) {
            unsetCursor();
            if (m_OldDragTrack > -1)
            {
                UpdateGeometry();
                m_Lane->paintTrack(m_OldDragTrack,Scene,zoomer->getZoom(),visibleRect(),0);
                m_OldDragTrack = -1;
            }
        }
        return QGraphicsView::event(e);
    }
    void showEvent(QShowEvent* e) {
        QGraphicsView::showEvent(e);
        zoomer->setZoom(zoomer->max());
        setZoom(zoomer->getZoom());
    }
    void resizeEvent(QResizeEvent* e) {
        QGraphicsView::resizeEvent(e);
        Paint();
    }
    void timerEvent(QTimerEvent* /*e*/) {
        if (!m_TimerID) return;
        m_TimeLine.handleTimer(m_Lane);
        if (m_Lane->requestIsPlaying()) {
            if (!visibleRect().contains(m_TimeLine.currentPos(),1)) {
                //horizontalScrollBar()->setValue(m_TimeLine.currentPos());
                m_Lane->requestSkip(m_TimeLine.sampleFromX(horizontalScrollBar()->value()));
            }
        }
    }
    void togglePlay() {
        if (m_Lane->requestIsPlaying()) {
            m_Lane->requestPause();
            return;
        }
        m_Lane->requestPlay(false);
    }
    void mouseDoubleClickEvent(QMouseEvent *event)
    {
        QPoint Pos=mapToScene(event->pos()).toPoint();
        UpdateGeometry();
        if (event->button()==Qt::LeftButton) {
            if (m_TimeLine.handleDoubleClick(Pos,m_Lane)) return;
        }
        int t = m_Lane->MouseOverTrack(Pos);
        if (!CurrentTrack->contains(t)) {
            CurrentTrack->clear();
            if (t > -1) CurrentTrack->append(t);
            emit Changed();
            Paint();
        }
    }
    void mousePressEvent(QMouseEvent *event)
    {
        QPoint Pos=mapToScene(event->pos()).toPoint();
        UpdateGeometry();
        if (event->button()==Qt::LeftButton) {
            if (m_TimeLine.handleMousePress(Pos,m_Lane)) return;
        }
        if (event->button()==Qt::RightButton) {
            QMenu* Popup = new QMenu(this);
            Popup->setAttribute(Qt::WA_DeleteOnClose);
            Popup->addActions(actionList);
            Popup->popup(mapToGlobal(event->pos()));
            return;
        }
        int t = m_Lane->MouseOverTrack(Pos);
        if (!CurrentTrack->contains(t)) {
            if (event->modifiers() != Qt::ControlModifier) CurrentTrack->clear();
            if (t > -1) CurrentTrack->append(t);
            emit Changed();
            Paint();
            if (t > -1) m_Lane->paintEdges(Pos,t,Scene,zoomer->getZoom(),visibleRect());
        }
        if (m_Lane->handleMousePress(Pos) > -1) m_Lane->drawOutsideWave(Scene,visibleRect());
    }
    void mouseMoveEvent(QMouseEvent *event)
    {
        QPoint Pos=mapToScene(event->pos()).toPoint();
        UpdateGeometry();
        if (m_TimeLine.handleMouseMove(Pos,m_Lane)) return;
        long64 s = m_Lane->handleMouseMove(Pos,&m_TimeLine);
        if (s > -1)
        {
            Paint();
            m_Lane->drawOutsideWave(Scene,visibleRect());
            for (const int& i : qAsConst(*CurrentTrack)) m_Lane->paintTrack(i,Scene,zoomer->getZoom(),visibleRect(),-1);
            return;
        }
        int TrackIndex=m_Lane->MouseOverTrack(Pos);
        if (TrackIndex > -1)
        {
            if ((TrackIndex != m_OldDragTrack) && (m_OldDragTrack > -1)) m_Lane->paintTrack(m_OldDragTrack,Scene,zoomer->getZoom(),visibleRect(),0);
            if (m_Lane->paintEdges(Pos,TrackIndex,Scene,zoomer->getZoom(),visibleRect())) {
                m_OldDragTrack = TrackIndex;
                setCursor(Qt::SizeHorCursor);
                return;
            }
        }
        unsetCursor();
        if (m_OldDragTrack > -1)
        {
            m_Lane->paintTrack(m_OldDragTrack,Scene,zoomer->getZoom(),visibleRect(),0);
            m_OldDragTrack = -1;
        }
    }
    void mouseReleaseEvent(QMouseEvent *event)
    {
        QPoint Pos=mapToScene(event->pos()).toPoint();
        UpdateGeometry();
        if (m_TimeLine.handleMouseRelease(Pos,m_Lane)) return;
        CWaveTrack* t = m_Lane->handleMouseRelease();
        for (const int& i : qAsConst(*CurrentTrack)) {
            if (CWaveTrack* t1 = m_Lane->tracks[i]) {
                m_Lane->sanityCheck(t1);
                if (!m_Lane->tracks[i]->isValid) CurrentTrack->removeOne(i);
            }
        }
        emit Changed();
        Paint();
        if (t) {
            int TrackIndex = m_Lane->tracks.indexOf(t);
            if (TrackIndex > -1) m_Lane->paintEdges(Pos,TrackIndex,Scene,zoomer->getZoom(),visibleRect());
        }
    }
    void zoomIn()
    {
        //ulong64 start=pos2Sample(viewPortGeometry().left());
        setZoom(zoomer->getZoom() * 2);
        //setViewportLeft(sample2Pos(start));
    }

    void zoomOut()
    {
        //ulong64 start=pos2Sample(viewPortGeometry().left());
        if (ldouble(sceneRect().width()) * 0.5L < viewport()->width())
        {
            setZoom(zoomer->getZoom() * ldouble(viewport()->width()-50)/ldouble(sceneRect().width()-50));
        }
        else
        {
            setZoom(zoomer->getZoom() * 0.5L);
        }
        //setViewportLeft(sample2Pos(start));
    }

    void zoomMin()
    {
        setZoom(zoomer->getZoom() * ldouble(viewport()->width()-50)/ldouble(sceneRect().width()-50));
        //setViewportLeft(0);
    }

    void zoomMax()
    {
        //ulong64 start=pos2Sample(visibleRect().left());
        setZoom(1);
        //setViewportLeft(sample2Pos(start));
    }
    void setZoom(double z) {
        if (z < 0) z = 1;
        if (zoomer->getZoom() != z) zoomer->setZoom(z);
        //m_TimeLine.setZoom(z);
        Paint();
        horizontalScrollBar()->setValue(qMax(0,m_TimeLine.currentPos()-(visibleRect().width()/2)));
    }
signals:
    void Changed();
private:
    Ui::CLaneEditControl *ui;
    QGraphicsScene Scene;
    CWaveLane* m_Lane;
    QGraphicsViewZoomer* zoomer;
    CTimeLine m_TimeLine;
    int m_TimerID = 0;
    QList<int>* CurrentTrack;
    int m_OldDragTrack = -1;
    QList<QAction*> actionList;
};

#endif // CLANEEDITCONTROL_H
