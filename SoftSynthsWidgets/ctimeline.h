#ifndef CTIMELINE_H
#define CTIMELINE_H

#include "idevice.h"
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsItemGroup>
#include "cpresets.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>

#define timelineheight 20
#define timelinehalfheight 10

class CTimeLine : protected IPresetRef
{
public:
    enum TimelineViews {
        TimeLineMilliseconds,
        TimelineBars
    };
    CTimeLine() {
        m_Samples = (presets.SampleRate* 60);
    }
    void setPen(QPen p) {
        m_Pen = p;
    }
    QPen pen() {
        return m_Pen;
    }
    void setOffset(int o) {
        m_Offset = o;
    }
    int offset() {
        return m_Offset;
    }
    void setWidth(int w) {
        m_Width = w;
        updateArrays();
    }
    int width() { return m_Width; }
    void setHeight(int h) {
        m_TimeLineHeight = h;
    }
    void setFixedWidth(int w, ulong64 s) {
        m_Width = w;
        m_Samples = s;
        if (m_Samples == 0) m_Samples = presets.SampleRate* 60;
        updateArrays();
    }
    int currentPos() const {
        return sampleToX(m_CurrentSample);
    }
    ulong currentmSec() const {
        return presets.samplesTomSecs(m_CurrentSample);
    }
    ulong64 currentSample() {
        return m_CurrentSample;
    }
    void setCurrentSample(const ulong64 s) {
        m_CurrentSample = s;
    }
    void setZoom(double z) {
        m_Zoom = z;
        updateArrays();
    }
    double zoom() { return m_Zoom; }
    void setSamples(ulong64 d) {
        m_Samples = d;
        if (m_Samples == 0) m_Samples = presets.SampleRate* 60;
        updateArrays();
    }
    ulong64 samples() { return  m_Samples; }
    ulong milliSeconds() { return  presets.samplesTomSecs(m_Samples); }
    int timeToX(ulong64 time) const {
        return sampleToX(presets.mSecsToSamples(time));
    }
    ulong64 timeFromX(int x) const {
        return presets.samplesTomSecs(sampleFromX(x));
    }
    int sampleToX(ulong64 sample) const {
        return ((ldouble(double(m_Width)*m_Zoom) / ldouble(m_Samples)) * ldouble(sample)) + m_Offset;
    }
    ulong64 sampleFromX(int x) const {
        return (ldouble(m_Samples) / ldouble(double(m_Width)*m_Zoom)) * ldouble(qMax(x - m_Offset,0));
    }
    TimelineViews view() {
        return m_View;
    }
    void setView(TimelineViews v) {
        m_View = v;
        updateArrays();
    }
    void setTimeSignature(int upper, int lower) {
        m_Upper = upper;
        m_Lower = lower;
    }
    void setTempo(double tempo) {
        m_Tempo = tempo;
    }
    int numberRow() {
        return m_NumberRow;
    }
    QString timeToText(ldouble t,TimelineViews v = TimeLineMilliseconds) {
        QString ret;
        switch (v) {
        case TimeLineMilliseconds:
            ret = mSecsToText(t);
            break;
        case TimelineBars:
            t += quarterrate/32.0;
            const int b = t / barrate;
            const int bt = fmod(t,barrate) / quarterrate;
            const int sb = fmod(t,quarterrate) / sixteensrate;
            const int sbb = fmod(t,sixteensrate) / sixtyfourrate;

            if (sbb) ret = QString::number(b+1) + ":" + QString::number(bt+1) + "(" + QString::number(sb+1)  + ":" + QString::number(sbb+1) + ")";
            else if (sb) ret = QString::number(b+1) + ":" + QString::number(bt+1) + "(" + QString::number(sb+1) + ")";
            else if (bt) ret = QString::number(b+1) + ":" + QString::number(bt+1);
            else ret = QString::number(b+1);
            break;
        }
        return ret;
    }
    QList<ldouble> lineUnits;
    void updateArrays() {
        quarterrate = ldouble((4*60000)/m_Lower)/m_Tempo;
        barrate = quarterrate * m_Upper;
        sixteensrate = quarterrate/4.0;
        sixtyfourrate = quarterrate/16.0;
        switch (m_View) {
        case TimeLineMilliseconds:
            lineUnits = lineArray(4);
            break;
        case TimelineBars:
            lineUnits = lineArray(16,{quarterrate/16.0, quarterrate/8.0, quarterrate/4.0, quarterrate/2.0, quarterrate, barrate, barrate*5, barrate*10,barrate*25,barrate*100});
            break;
        }
    }
    QList<ldouble> lineArray(const double minDistance, const QList<ldouble>& tabs = {1,10,100,1000,10000,60000,600000,1800000,3600000}) {
        QList<ldouble> ret;
        const ldouble endtime = ldouble(m_Samples)/presets.SamplesPermSec;
        const ldouble unit = endtime / ((m_Width*m_Zoom)/minDistance);
        ret.append(tabs.last());
        for (int i = tabs.size()-2; i >= 0; i--) {
            if (unit < tabs[i]) {
                ret.append(tabs[i]);
            }
        }
        m_NumberRow = 0;
        for (int i = ret.size()-1; i >= 0; i--)
        {
            if (timeToX(ret[i]) > 40) {
                m_NumberRow = i;
                break;
            }
        }
        return ret;
    }
    void render(QGraphicsScene* Scene, QRect visibleRect = QRect()) {
        if (visibleRect == QRect()) {
            visibleRect.setWidth(m_Width + m_Offset);
            visibleRect.setHeight(m_TimeLineHeight);
        }
        if (m_TimeLineHeight > 0) {
            QFont f = Scene->font();
            f.setPointSizeF(9);
            QColor c = m_Pen.color();
            QList<ldouble> l = lineUnits;
            const ldouble endtime = ldouble(m_Samples)/presets.SamplesPermSec;
            const double h = double(m_TimeLineHeight) / double(l.size());
            double bottom = h;
            if (m_Width > 0) {
                for (int i = l.size() - 1; i >= 0; i--) {
                    int count = 0;
                    ldouble t = 0;
                    while (t < endtime) {
                        t = l[i] * count++;
                        const int x = timeToX(t);
                        if (visibleRect.contains(x,2)) {
                            Scene->addLine(x,0,x,bottom,m_Pen);
                            if (i == numberRow())
                            {
                                QGraphicsTextItem* i = Scene->addText(timeToText(t,m_View),f);
                                i->setDefaultTextColor(c);
                                i->setPos(x,timelinehalfheight);
                            }
                        }
                    }
                    bottom += h;
                }
            }
            m_PlayLine = new QGraphicsItemGroup();
            m_PlayLine->addToGroup(Scene->addLine(0,0,0,visibleRect.height(),QPen(Qt::yellow)));
            m_PlayLine->addToGroup(Scene->addLine(1,0,1,visibleRect.height(),QPen(QColor(0,0,0,40))));
            m_PlayLine->setVisible(true);
            m_PlayLine->setZValue(1);
            Scene->addItem(m_PlayLine);
            m_PlayLine->setPos(sampleToX(m_CurrentSample),0);
        }
    }
    void handleTimer(IDevice* d) {
        if (m_TimeLineHeight > 0) {
            if (d->requestIsPlaying()) {
                ulong64 s = d->requestCurrentSample();
                int p = sampleToX(s);
                if (p != currentPos()) {
                    m_CurrentSample = s;
                    m_PlayLine->setPos(p,0);
                }
            }
        }
    }
    bool handleDoubleClick(QPointF p, IDevice* d) {
        if (p.y() < m_TimeLineHeight) {
            if (d->requestIsPlaying()) {
                d->requestPause();
            }
            else {
                d->requestSkip(sampleFromX(p.x()));
                d->requestPlay(false);
            }
            return true;
        }
        return false;
    }
    bool handleMousePress(QPointF p, IDevice* d) {
        if (p.y() < m_TimeLineHeight) {
            d->requestSkip(sampleFromX(p.x()));
            skip(sampleFromX(p.x()));
            m_MD = true;
            return true;
        }
        return false;
    }
    bool handleMouseMove(QPointF p, IDevice* d) {
        if (m_MD) {
            if (currentPos() != p.x()) {
                d->requestSkip(sampleFromX(p.x()));
                skip(sampleFromX(p.x()));
            }
            return true;
        }
        return false;
    }
    bool handleMouseRelease(QPointF p, IDevice* d) {
        if (m_MD) {
            m_MD = false;
            if (currentPos() != p.x()) {
                d->requestSkip(sampleFromX(p.x()));
                skip(sampleFromX(p.x()));
            }
            return true;
        }
        return false;
    }
    void skip(const ulong64 samples) {
        int p = sampleToX(samples);
        if (samples != m_CurrentSample) {
            m_CurrentSample = samples;
            if (m_TimeLineHeight > 0) m_PlayLine->setPos(p,0);
        }
    }
    void serialize(QDomLiteElement* xml) const {
        if (QDomLiteElement* e = xml->appendChild("TimeLine")) {
            e->setAttribute("Tempo",m_Tempo);
            e->setAttribute("Upper",m_Upper);
            e->setAttribute("Lower",m_Lower);
            e->setAttribute("View",m_View);
        }
    }
    void unserialize(const QDomLiteElement* xml) {
        if (QDomLiteElement* e = xml->elementByTag("TimeLine")) {
            m_Tempo = e->attributeValue("Tempo",90);
            m_Upper = e->attributeValueInt("Upper",4);
            m_Lower = e->attributeValueInt("Lower",4);
            m_View = (TimelineViews)e->attributeValueInt("View",TimeLineMilliseconds);
            updateArrays();
        }
    }
private:
    QPen m_Pen = QPen(Qt::white);
    QGraphicsItemGroup* m_PlayLine;
    int m_Width=100;
    int m_Offset = 0;
    double m_Zoom=1;
    ulong64 m_Samples;
    TimelineViews m_View=TimeLineMilliseconds;
    int m_Upper = 4;
    int m_Lower = 4;
    double m_Tempo = 90;
    int m_NumberRow = 0;
    bool m_MD = false;
    ulong64 m_CurrentSample = 0;
    int m_TimeLineHeight = timelineheight;
    ldouble quarterrate;
    ldouble barrate;
    ldouble sixteensrate;
    ldouble sixtyfourrate;
};

#endif // CTIMELINE_H
